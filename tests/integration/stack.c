#define LIBFUN_PREFIX t

#define T long int, another_type
#include "../../include/stack.h"

#define T int, int
#include "../../include/stack.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>


void test_resize(void)
{
	struct tstack_int s;

	tstack_int_xwith_cap(&s, 0);
	assert(tstack_int_reserve(&s, 0) == 0)  /* noop */;

	/* reserve capacity beforehand */
	assert(tstack_int_reserve(&s, 10) == 0);
	size_t cap = tstack_int_cap(&s);

	for (int i = 0; i < 10; i++)
		tstack_int_xpush(&s, &(int) { i });

	/* no reallocation should occur */
	assert(cap == tstack_int_cap(&s));

	tstack_int_xpush(&s, &(int) { 10 });
	assert(cap < tstack_int_cap(&s));

	tstack_int_shrink_to_fit(&s);
	assert(tstack_int_cap(&s) == 11);

	const int *popped = tstack_int_multipop(&s, 10);
	for (int i = 0; i < 10; i++)
		assert(popped[i] == i + 1);
	assert(*tstack_int_pop(&s) == 0);

	/* has 0, reserve for 1, but cap is already 11 */
	assert(tstack_int_reserve(&s, 1) == 0);
	assert(tstack_int_cap(&s) == 11);

	tstack_int_destroy(&s);
}

void test_multi(void)
{
	struct tstack_int s;
	tstack_int_xwith_cap(&s, 0);

	int values[64];

	for (size_t i = 0; i < 64; i++)
		values[i] = rand();

	const int *pushed = tstack_int_xmultipush(&s, values, 32);
	for (size_t i = 0; i < 32; i++)
		assert(pushed[i] == values[i]);

	const int *popped = tstack_int_multipop(&s, 16);
	for (size_t i = 0; i < 16; i++)
		assert(popped[i] == values[i + 16]);

	tstack_int_xmultipush(&s, &values[32], 32);
	for (size_t i = 0; i < 32; i++)
		assert(*tstack_int_pop(&s) == values[63 - i]);

	tstack_int_destroy(&s);
}

void test_fuzz(void)
{
	struct tstack_int s;

	tstack_int_xwith_cap(&s, 0);
	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		int limit = rand() % 1024;
		for (int i = 0; i < limit; i++) {
			tstack_int_xpush(&s, &i);

			assert(*tstack_int_at(&s, i) == i);
			assert(*tstack_int_peek(&s, 0) == i);
			assert(tstack_int_len(&s) == (size_t) i + 1);
		}

		for (int i = 0; i < limit; i++) {
			assert(*tstack_int_at(&s, i) == i);
			assert(*tstack_int_peek(&s, i) == limit - i - 1);
		}

		for (int i = limit; i > 0; i--) {
			assert(*tstack_int_top(&s) == i - 1);
			assert(*tstack_int_pop(&s) == i - 1);

			int *item = tstack_int_xpush(&s, NULL);
			*item = i;
			assert(*tstack_int_pop(&s) == i);
		}

		tstack_int_clear(&s);
	}
	tstack_int_destroy(&s);
}


int main(void)
{
	srand(time(NULL));

	test_resize();
	test_multi();
	test_fuzz();
}
