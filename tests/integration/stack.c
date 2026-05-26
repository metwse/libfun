#include "../../include/stack.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>


int main(void)
{
	srand(time(NULL));

	struct lf(stack) s;

	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		lf(stack_xinit)(&s, sizeof(int));

		int limit = rand() % 1024;
		for (int i = 0; i < limit; i++) {
			lf(stack_xpush)(&s, &i);
			assert(*(int *) lf(stack_at)(&s, i) == i);
			assert(*(int *) lf(stack_at)(&s, -1) == i);
			assert(lf(stack_len)(&s) == (size_t) i + 1);
		}
		for (int i = 1; i <= limit; i++) {
			assert(*(int *) lf(stack_at)(&s, -i) == limit - i);
		}

		for (int i = limit; i > 0; i--) {
			assert(*(int *) lf(stack_top)(&s) == i - 1);
			assert(*(int *) lf(stack_pop)(&s) == i - 1);
		}

		lf(stack_destroy)(&s);
	}

	return EXIT_SUCCESS;
}
