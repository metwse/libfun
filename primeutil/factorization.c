/*!
 * Calculates the prime factors of a number.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PRIME_CACHE_BLOCK 128
#define PRIME_FACTORS_BLOCK 16

static uint64_t *prime_cache = 0;
static size_t prime_cache_cap = 128;
static size_t prime_cache_len = 1;

struct prime_list {
	uint64_t *list;
	size_t len;
};

bool is_prime(uint64_t num) {
	if (num <= 1)
		return false;

	for (size_t i = 0; prime_cache[i] * prime_cache[i] <= num; i++) {
		if (!(num % prime_cache[i]))
			return false;
	}
	return true;
}

void push_prime(uint64_t prime) {
	if (prime_cache_cap == prime_cache_len) {
		prime_cache = realloc(prime_cache,
			sizeof(uint64_t) * \
			(prime_cache_cap + PRIME_CACHE_BLOCK));
		prime_cache_cap += PRIME_CACHE_BLOCK;
	}

	prime_cache[prime_cache_len] = prime;
	prime_cache_len++;
}

void prime_list_push_prime(struct prime_list *l, uint64_t num, size_t *l_cap) {
	if (*l_cap == l->len) {
		l->list = realloc(l->list,
		    sizeof(uint64_t) * \
		    (*l_cap + PRIME_FACTORS_BLOCK));
		*l_cap += PRIME_FACTORS_BLOCK;
	}

	l->list[l->len] = num;
	l->len++;
}

void prime_list_print(struct prime_list *l) {
	printf("[");
	for (size_t i = 0; i < l->len; i++)
		printf(i == l->len - 1 ? "%ld]" : "%ld, ", l->list[i]);
}

struct prime_list *find_factors(uint64_t num) {
	if (!prime_cache) {
		prime_cache = malloc(sizeof(uint64_t) * PRIME_CACHE_BLOCK);
		prime_cache[0] = 2;
	}

	struct prime_list *l = malloc(sizeof(struct prime_list));
	l->len = 0;

	if (num <= 1)
		return l;

	size_t l_cap = PRIME_FACTORS_BLOCK;
	l->list = malloc(PRIME_FACTORS_BLOCK * sizeof(uint64_t));

	for (size_t i = 0; i < prime_cache_len; i++) {
		uint64_t prime = prime_cache[i];

		if (prime * prime > num)
			break;

		if (!(num % prime)) {
			prime_list_push_prime(l, prime, &l_cap);

			do
				num /= prime;
			while (!(num % prime));
		}
	}

	for (size_t i = prime_cache[prime_cache_len - 1] + 1; i <= num; i++) {
		if (is_prime(i)) {
			push_prime(i);

			if (!(num % i)) {
				prime_list_push_prime(l, i, &l_cap);
				do
					num /= i;
				while (!(num % i));
			}
		}
	}

	return l;
}

int main() {
	uint64_t num;
	printf("Please entar a positive number: ");
	scanf("%lu", &num);

	struct prime_list *list = find_factors(num);
	printf("Prime factors of %lu are ", num);
	prime_list_print(list);
	printf(".\n");

	return 0;
}
