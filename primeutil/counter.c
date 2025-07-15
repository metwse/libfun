/*!
 * Returns an estimate of π(N), the number of primes ≤ N.
 */

#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_COUNT 16
#define CALCULATION_CHUNK 65536

// A thread goes to sleep if the difference in iteration count between itself
// and the thread with the minimum iterations exceeds this threshold.
#define SLEEP_TRESH 128

// When the difference drops below this threshold, the sleeping thread resumes
// prime counting.
#define WAKE_TRESH 64


struct calcuation_pool {
	pthread_mutex_t locks[THREAD_COUNT];
	long double prime_count[THREAD_COUNT];
	unsigned long long chunk_iterations[THREAD_COUNT];

#ifdef SYNC_THREAD_ITERATIONS
	pthread_mutex_t min_itearation_lock;
	pthread_cond_t min_itearation_cv;
	unsigned long long min_itearation_owner;

	unsigned long long min_itearation;
#endif
};

struct solver_thread_args {
	unsigned long long thread_id;
	struct calcuation_pool *pool;
};

struct screen_thread_args {
	const char *savefile;
	struct calcuation_pool *pool;
};


void *solver_thread(void *);
void *screen_thread(void *);
void clear_screen();


int main(int argc, char **argv)
{
	char *data_filename = NULL;
	if (argc == 2)
		data_filename = argv[1];

	struct calcuation_pool pool = {};
	for (int i = 0; i < THREAD_COUNT; i++)
		pthread_mutex_init(&pool.locks[i], NULL);

#ifdef SYNC_THREAD_ITERATIONS
	pthread_mutex_init(&pool.min_itearation_lock, NULL);
	pthread_cond_init(&pool.min_itearation_cv, NULL);
#endif

	pthread_t solver_thread_join[THREAD_COUNT];
	pthread_t screen_thread_join;
	struct solver_thread_args args[THREAD_COUNT];
	for (unsigned long long i = 0; i < THREAD_COUNT; i++) {
		args[i].thread_id = i;
		args[i].pool = &pool;

		pthread_create(&solver_thread_join[i], NULL, *solver_thread,
			       &args[i]);
	}

	bool data_loaded = false;
	if (argc != 2) {
		printf("WARNING: No save file specified.\n");
	} else {
		FILE *datafile = fopen(data_filename, "rb");
		if (datafile == NULL) {
			printf("Could not open %s\n", data_filename);
		} else {
			printf("Loading data from %s\n", data_filename);

			data_loaded = fread(&pool.chunk_iterations,
				     sizeof(pool.chunk_iterations),
				     1,
				     datafile) == 1;
			if (data_loaded)
				data_loaded = fread(&pool.prime_count,
					     sizeof(pool.prime_count),
					     1,
					     datafile) == 1;
			if (!data_loaded)
				printf("WARNING: %s is corrupted\n",
				       data_filename);

			fclose(datafile);
		}
	}

	if (!data_loaded)
		for (int i = 0; i < THREAD_COUNT; i++) {
			pool.prime_count[i] = 0;
			pool.chunk_iterations[i] = 0;
		}
#ifdef SYNC_THREAD_ITERATIONS
	else {
		pool.min_itearation = 0;
		pool.min_itearation_owner = 0;
		for (int i = 1; i < THREAD_COUNT; i++)
			if (pool.min_itearation > pool.chunk_iterations[i]) {
				pool.min_itearation = pool.chunk_iterations[i];
				pool.min_itearation_owner = i;
			}
	}
#endif

	struct screen_thread_args screen_thread_args = {
		.pool = &pool,
		.savefile = data_filename,
	};

	pthread_create(&screen_thread_join, NULL, *screen_thread,
		       &screen_thread_args);

	for (int i = 0; i < THREAD_COUNT; i++)
		pthread_join(solver_thread_join[i], NULL);
	pthread_join(screen_thread_join, NULL);

	return EXIT_SUCCESS;
}

void *screen_thread(void *_arg)
{
	struct screen_thread_args *args = _arg;
	struct calcuation_pool *pool = args->pool;
	const char *save_filename = args->savefile;

	long double sync_prime_count[THREAD_COUNT];
	unsigned long long sync_chunk_iterations[THREAD_COUNT];
	long double total_prime_count;
	unsigned long long total_iterations;
	unsigned long long diff_total_iterations = 0;

	while (true) {
		usleep(1e6);

		for (int i = 0; i < THREAD_COUNT; i++) {
			pthread_mutex_t *lock = &pool->locks[i];

			pthread_mutex_lock(lock);
			sync_prime_count[i] = pool->prime_count[i];
			sync_chunk_iterations[i] = pool->chunk_iterations[i];
			pthread_mutex_unlock(lock);
		}

		clear_screen();
		printf("\tTHREAD STATS\nthread_id\titeration_count\tprime_count\n");
		total_iterations = 0;
		total_prime_count = 0;
		for (int i = 0; i < THREAD_COUNT; i++) {
			printf("%d\t\t%-12llu\t%.2Lf\n",
			       i, sync_chunk_iterations[i], sync_prime_count[i]);
			total_iterations += sync_chunk_iterations[i];
			total_prime_count += sync_prime_count[i];
		}

		total_iterations *= CALCULATION_CHUNK;
		long int iteration_digit_count = floorl(log10l(total_iterations));

		printf("\n\tTOTAL\n");
		printf("prime_count: %.2Lf (%.2Lf MiB)\n",
		       total_prime_count, total_prime_count * 8 / 1024 / 1024);
		printf("checked_numbers: %lld (%.2lfe%ld)\n", total_iterations,
		       (total_iterations / (pow(10, iteration_digit_count - 1))) / 10.0,
		       iteration_digit_count);
		printf("prime_frequency: %Lf\n",
		       total_prime_count / total_iterations);
		printf("checks_per_second: %lld\n",
		       total_iterations - diff_total_iterations);

		diff_total_iterations = total_iterations;

		if (save_filename != NULL) {
			FILE *datafile = fopen(save_filename, "wb");

			fwrite(&sync_chunk_iterations, sizeof(sync_chunk_iterations), 1, datafile);
			fwrite(&sync_prime_count, sizeof(sync_prime_count), 1, datafile);

			fclose(datafile);
		}
	}

	return NULL;
}

void *solver_thread(void *_args)
{
	struct solver_thread_args *args = (struct solver_thread_args *) _args;
	struct calcuation_pool *pool = args->pool;
	unsigned long long thread_id = args->thread_id;

	pthread_mutex_t *lock = &pool->locks[thread_id];
	long double *prime_count = &pool->prime_count[thread_id];
	unsigned long long *chunk_iterations = &pool->chunk_iterations[thread_id];

	unsigned long long num = 2 + thread_id * CALCULATION_CHUNK;

	long double total_nums;

#ifdef SYNC_THREAD_ITERATIONS
	bool will_sleep = false;
#endif

	while (true) {
#ifdef SYNC_THREAD_ITERATIONS
		if (will_sleep) {
			pthread_mutex_lock(&pool->min_itearation_lock);
			pthread_cond_wait(&pool->min_itearation_cv,
					  &pool->min_itearation_lock);
			will_sleep = *chunk_iterations > pool->min_itearation + WAKE_TRESH;
			pthread_mutex_unlock(&pool->min_itearation_lock);
			continue;
		}
#endif

		total_nums = 0;

		for (unsigned long long i = 0; i < CALCULATION_CHUNK; i++) {
			total_nums += 1 / logl(num);
			num++;
		}
		num += THREAD_COUNT * CALCULATION_CHUNK;

		pthread_mutex_lock(lock);

		*prime_count += total_nums;
		(*chunk_iterations)++;

#ifdef SYNC_THREAD_ITERATIONS
		pthread_mutex_lock(&pool->min_itearation_lock);

		if (pool->min_itearation_owner == thread_id) {
			pool->min_itearation = *chunk_iterations;
			pthread_cond_broadcast(&pool->min_itearation_cv);
		} else if (pool->min_itearation > *chunk_iterations) {
			pool->min_itearation = *chunk_iterations;
			pool->min_itearation_owner = thread_id;
			pthread_cond_broadcast(&pool->min_itearation_cv);
		} else if (*chunk_iterations - pool->min_itearation > SLEEP_TRESH){
			will_sleep = true;
		}

		pthread_mutex_unlock(&pool->min_itearation_lock);
#endif
		pthread_mutex_unlock(lock);
	}

	return NULL;
}

void clear_screen()
{
	fwrite("\e[1;1H\e[2J", 10, 1, stdout);
}
