#include "../../include/vector.h"

#include <assert.h>
#include <stdlib.h>


void t_vec_creation()
{
	struct vec_result init_result;

	for (int i = 0; i < 1000; i++) {
		init_result = vec_init();

		assert(init_result.v == VECOK_VEC);
		assert(init_result.d.vec);

		vec_destroy(init_result.d.vec);
	}

	for (int i = 0; i < 1000; i++) {
		init_result = vec_with_capacity(rand() % 100);

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

	for (int i = 0; i < 1000; i++) {
		int new_size = rand() % 100 + 1;
		result = vec_expand(vec, new_size);
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

	for (int i = 0; i < 10; i++) {
		int list[100];
		for (int j = 0; j < 100; j++) {
			int *r = malloc(sizeof(int));
			list[j] = *r = rand() % 100 + 1;

			result = vec_append(vec, r);
			assert(result.v == VECOK);
		}

		result = vec_shrink_to_fit(vec);
		assert(result.v == VECOK_VEC);
		assert(result.d.vec == NULL);

		result = vec_shrink(vec, 50);
		assert(result.v == VECOK_VEC);
		assert(result.d.vec);
		assert(result.d.vec->size == 50);
		following_part = result.d.vec;


		for (int j = 0; j < 50; j++) {
			result = vec_get(following_part, j);
			assert(result.v == VECOK_ELEMENT);
			assert(list[j + 50] == *((int *) result.d.element));

			free(result.d.element);
		}

		for (int j = 0; j < 25; j++) {
			result = vec_remove(vec, 49 - j);
			assert(result.v == VECOK_ELEMENT);
			assert(list[49 - j] == *((int *) result.d.element));

			free(result.d.element);
		}

		for (int j = 0; j < 25; j++) {
			result = vec_pop(vec);
			assert(result.v == VECOK_ELEMENT);
			assert(list[24 - j] == *((int *) result.d.element));

			free(result.d.element);
		}

		vec_destroy(following_part);
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
