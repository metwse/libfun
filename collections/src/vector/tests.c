#include "../../include/vector.h"

#include <assert.h>
#include <stdlib.h>


void t_vec_creation()
{
	struct vec_result init_result;

	for (int i = 0; i < 1024; i++) {
		init_result = vec_init();

		assert(init_result.v == VECOK_VEC);
		assert(init_result.d.vec);

		vec_destroy(init_result.d.vec);
	}

	for (int i = 0; i < 1024; i++) {
		init_result = vec_with_capacity(rand() % 96);

		assert(init_result.v == VECOK_VEC);
		assert(init_result.d.vec);

		vec_destroy(init_result.d.vec);
	}
}

void t_vec_resize()
{
	struct vec *vec = vec_init().d.vec;
	struct vec_result result;
	struct vec *following_part;

	for (int i = 0; i < 1024; i++) {
		int new_size = rand() % 96 + VEC_EXPAND_DELTA + 1;
		result = vec_expand(vec, new_size);
		if (result.v == VECE && result.d.err == VECE_NOMEM) {
			i--;
			continue;
		}
		assert(result.v == VECOK);

		result = vec_expand(vec, new_size - 1);
		assert(result.v == VECE);
		assert(result.d.err == VECE_CAP_IS_GREATER);

		result = vec_shrink(vec, 0);
		assert(result.v == VECOK_VEC);
		assert(!result.d.vec);

		result = vec_shrink(vec, 1);
		assert(result.v == VECE);
		assert(result.d.err == VECE_CAP_IS_LOWER);
	}

	for (int i = 0; i < 16; i++) {
		int list[128];
		for (int j = 0; j < 128; j++) {
			int *r = malloc(sizeof(int));
			list[j] = *r = rand();

			result = vec_append(vec, r);
			if (result.v == VECE && result.d.err == VECE_NOMEM) {
				i--;
				continue;
			}
			assert(result.v == VECOK);
		}

		result = vec_shrink_to_fit(vec);
		assert(result.v == VECOK_VEC);
		assert(result.d.vec == NULL);

		result = vec_shrink(vec, 64);
		assert(result.v == VECOK_VEC);
		assert(result.d.vec);
		assert(result.d.vec->size == 64);
		following_part = result.d.vec;

		for (int j = 0; j < 64; j++) {
			result = vec_get(following_part, j);
			assert(result.v == VECOK_ELEMENT);
			assert(list[j + 64] == *((int *) result.d.element));

			free(result.d.element);
		}
		vec_destroy(following_part);

		for (int j = 0; j < 32; j++) {
			result = vec_remove(vec, 63 - j);
			assert(result.v == VECOK_ELEMENT);
			assert(list[63 - j] == *((int *) result.d.element));

			free(result.d.element);
		}

		for (int j = 0; j < 16; j++) {
			result = vec_pop(vec);
			assert(result.v == VECOK_ELEMENT);
			assert(list[31 - j] == *((int *) result.d.element));

			free(result.d.element);
		}

		int *dummy;
		for (int j = 0; j < 16; j++) {
			result = vec_replace(vec, j, dummy);
			assert(result.v == VECOK_ELEMENT);
			assert(list[j] == *((int *) result.d.element));

			free(result.d.element);
		}

		for (int j = 0; j < 16; j++)
			result = vec_pop(vec);
	}

	vec_destroy(vec);
}

#ifdef _TEST_VEC_MAIN
#include <time.h>
int main()
{
	srand(time(NULL));
	t_vec_creation();
	t_vec_resize();
}
#endif
