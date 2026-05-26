#include "../../include/map.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


int int_comparator(const void *key1_, const void *key2_,
		   size_t keylen1, size_t keylen2) {
	(void) keylen1; (void) keylen2;

	int key1, key2;

	key1 = *(int *) key1_;
	key2 = *(int *) key2_;

	if (key1 < key2)
		return -1;
	else if (key1 == key2)
		return 0;
	else
		return 1;
}

int main(void)
{
	struct lf(map) m;

	lf(map_xinit)(&m, 0, int_comparator);

	for (int i = 2048; i < 4096; i++)
		lf(map_xinsert2(&m, &i, sizeof(int), NULL));

	for (int i = 2047; i > -1; i--)
		lf(map_xinsert2(&m, &i, sizeof(int), NULL));

	struct lf(map_it) it;

	lf(map_iter)(&m, &it);

	for (int i = 0; i < 3072; i++) {
		struct lf(map_entry) e = lf(map_iter_next)(&it);

		assert(memcmp(&e.key, &i, sizeof(int)));
	}

	for (int i = 3072; i > 1024; i--) {
		struct lf(map_entry) e = lf(map_iter_prev)(&it);

		assert(memcmp(&e.key, &i, sizeof(int)));
	}

	for (int i = 0; i < 4096; i += 3)
		assert(lf(map_remove2)(&m, &i, sizeof(int)));

	lf(map_iter_rev)(&m, &it);

	for (int i = 4095; i > -1; i--) {
		if (i % 3 == 0) {
			assert(lf(map_get2(&m, &i, sizeof(int)) == NULL));
			assert(lf(map_rank2(&m, &i, sizeof(int)) == (size_t) -1));
		} else {
			struct lf(map_entry) e = lf(map_iter_prev)(&it);

			assert(memcmp(&e.key, &i, sizeof(int)));
		}
	}

	lf(map_destroy)(&m);

	return EXIT_SUCCESS;
}
