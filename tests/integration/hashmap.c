#include "../../include/hashmap.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(void)
{
	srand(time(NULL));

	struct lf(hashmap) m;

	for (int _fuzz = 0; _fuzz < 128; _fuzz++) {
		int elem_size = rand() % 32;

		lf(hashmap_xinit)(&m, elem_size);

		char value[elem_size + 1];

		for (int i = 0; i <= elem_size; i++)
			value[i] = rand() % 255;

		int limit = rand() % 4096;
		char values[limit];

		size_t values_total = 0;

		for (int i = 0; i < limit; i++) {
			value[0] = rand() % 255;

			values[i] = value[0];

			// hashmap does not contain the key
			assert(!lf(hashmap_get2)(&m, &i, sizeof(int)));

			lf(hashmap_xinsert2)(&m, &i, sizeof(int), value);

			// now it contain
			assert(lf(hashmap_get2)(&m, &i, sizeof(int)));

			assert(!memcmp(lf(hashmap_get2)(&m, &i, sizeof(int)),
							value, elem_size));

			values_total += value[0];
		}

		if (elem_size) {
			size_t iter_values_total = 0;

			struct lf(hashmap_it) it;

			lf(hashmap_iter)(&m, &it);

			struct lf(entry) e;
			while (lf(entry_is_valid)(e = lf(hashmap_iter_next)(&it)))
				iter_values_total += ((char *) e.value)[0];

			assert(iter_values_total == values_total);
		}

		int limit2 = limit % ((rand() % 2048) + 4);
		// remove all even-numbered keys
		for (int i = 0; i < limit2; i += 2) {
			value[0] = values[i];

			// check element
			assert(!memcmp(lf(hashmap_get2)(&m, &i, sizeof(int)),
							value, elem_size));

			// now remove it
			assert(!memcmp(lf(hashmap_remove2)(&m, &i, sizeof(int)),
							   value, elem_size));

			// the key should not present
			assert(!lf(hashmap_get2)(&m, &i, sizeof(int)));
		}

		int limit3 = limit2 % ((rand() % 1024) + 4);
		// now insert size_t keyed elements
		for (size_t i = 0; i < (size_t) limit3; i += 4)
			lf(hashmap_xinsert2)(&m, &i, sizeof(size_t), &i);

		// check size_t keyed elements
		size_t inserted_size = sizeof(size_t);

		if ((size_t) elem_size < inserted_size)
			inserted_size = elem_size;

		for (size_t i = 0; i < (size_t) limit3; i += 4)
			assert(!memcmp(lf(hashmap_get2)(&m, &i, sizeof(size_t)),
							&i, inserted_size));

		lf(hashmap_xinsert)(&m, "inserting string", "the string value");
		assert(lf(hashmap_get)(&m, "inserting string"));
		assert(!strncmp(lf(hashmap_remove)(&m, "inserting string"),
						   "the string value",
						   elem_size));
		assert(!lf(hashmap_remove)(&m, "inserting string"));

		lf(hashmap_destroy)(&m);
	}

	return EXIT_SUCCESS;
}
