#define LIBFUN_PREFIX t

#define T long int, another_type
#include "../../include/stack.h"

#define T int, int
#include "../../include/stack.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>


int main(void)
{
	srand(time(NULL));

	struct tstack_int s;

	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		tstack_int_xinit(&s);

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

	return EXIT_SUCCESS;
}
