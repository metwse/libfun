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

	tstack_int_xpush(&s, &(int) { 1 });
	assert(cap < tstack_int_cap(&s));

	tstack_int_shrink_to_fit(&s);
	assert(tstack_int_cap(&s) == 11);

	for (int i = 0; i < 11; i++)
		tstack_int_pop(&s);
	/* has 0, reserve for 1, but cap is already 11 */
	assert(tstack_int_reserve(&s, 1) == 0);
	assert(tstack_int_cap(&s) == 11);

	tstack_int_destroy(&s);
}

void test_fuzz(void)
{
	struct tstack_int s;

	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		tstack_int_xwith_cap(&s, 0);

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

		tstack_int_destroy(&s);
	}
}


int main(void)
{
	srand(time(NULL));

	test_resize();
	test_fuzz();
}
