#include "../../include/map.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


int main(void)
{
	struct lf(map) m;

	for (int dir = 0; dir < 2; dir++) {
		lf(map_xinit)(&m, sizeof(int), NULL);

		char str[4] = { '0', '0', '0', '\0' };

		int dir3 = dir;

		for (int dir2 = 0; dir2 < 2; dir2++) {
			if (dir3) {
				str[2] = '0';
				for (int i = 0; i < 5; i++) {
					str[0] = '0';
					for (int j = 0; j < 10; j++) {
						str[1] = '0';
						for (int k = 0; k < 10; k++) {
							int d = strtod(str, NULL);

							lf(map_xinsert)(&m, str, &d);
							str[1]++;
						}
						str[0]++;
					}
					str[2]++;
				}
			} else {
				str[2] = '9';
				for (int i = 10; i > 5; i--) {
					str[0] = '0';
					for (int j = 0; j < 10; j++) {
						str[1] = '0';
						for (int k = 0; k < 10; k++) {
							int d = strtod(str, NULL);

							lf(map_xinsert)(&m, str, &d);
							str[1]++;
						}
						str[0]++;
					}
					str[2]--;
				}
			}

			dir3 = !dir3;
		}

		str[0] = '0';

		for (int i = 0; i < 10; i++) {
			str[1] = '0';
			for (int j = 0; j < 10; j++) {
				str[2] = '0';
				for (int k = 0; k < 10; k++) {
					int i = strtod(str, NULL);

					assert(memcmp(&i, lf(map_get)(&m, str),
		   sizeof(int)) == 0);

					str[2]++;
				}
				str[1]++;
			}
			str[0]++;
		}

		for (int i = 0; i < 1000; i++) {
			int rev_i = 999 - i;

			struct lf(map_entry) e = lf(map_select)(&m, i);
			struct lf(map_entry) rev_e = lf(map_select)(&m, -(i + 1));

			assert(memcmp(&i, e.value, sizeof(int)) == 0);
			assert(memcmp(&rev_i, rev_e.value, sizeof(int)) == 0);

			memcpy(str, e.key, 3);

			assert(lf(map_rank)(&m, str) == (size_t) i);
		}

		assert(lf(map_rank(&m, "1000")) == (size_t) -1);

		str[0] = '0'; str[1] = '0'; str[2] = '0';
		for (int i = 0; i < 10; i++) {
			int *value = lf(map_remove(&m, str));

			int key = strtod(str, NULL);

			assert(memcmp(&key, value, sizeof(int)) == 0);

			str[0]++; str[1]++; str[2]++;
		}

		str[2] = '9';
		for (int i = 0; i < 10; i++) {
			str[0] = '0';
			for (int j = 0; j < 10; j++) {
				str[1] = '0';
				for (int k = 0; k < 10; k++) {
					if (str[1] == str[2] && str[2] == str[0]) {
						assert(lf(map_get)(&m, str) == NULL);
					} else {
						int key = strtod(str, NULL);
						assert(memcmp(&key,
						       lf(map_get)(&m, str),
						       sizeof(int)) == 0);
					}

					str[1]++;
				}
				str[0]++;
			}
			str[2]--;
		}

		lf(map_destroy)(&m);
	}

	for (int _fuzz = 0; _fuzz < 256; _fuzz++) {
		int elem_size = rand() % 32;

		lf(map_xinit)(&m, elem_size, NULL);

		for (int i = 0; i < 1024; i++) {
			if (lf(map_get2)(&m, &i, sizeof(int)) == NULL)
				lf(map_xinsert2)(&m, &i, sizeof(int), &i);
		}

		lf(map_destroy)(&m);
	}

	return EXIT_SUCCESS;
}
