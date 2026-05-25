#include "../../include/map.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


int main(void)
{
	struct map m;

	map_init(&m, sizeof(int), NULL);

	char str[4] = { '0', '0', '0', '\0' };

	for (int i = 0; i < 10; i++) {
		str[0] = '0';
		for (int j = 0; j < 10; j++) {
			str[1] = '0';
			for (int k = 0; k < 10; k++) {
				int i = strtod(str, NULL);

				map_insert2(&m, str, 3, &i);
				str[1]++;
			}
			str[0]++;
		}
		str[2]++;
	}

	str[0] = '0';

	for (int i = 0; i < 10; i++) {
		str[1] = '0';
		for (int j = 0; j < 10; j++) {
			str[2] = '0';
			for (int k = 0; k < 10; k++) {
				int i = strtod(str, NULL);

				assert(memcmp(&i, map_get2(&m, str, 3),
					      sizeof(int)) == 0);

				str[2]++;
			}
			str[1]++;
		}
		str[0]++;
	}

	for (int i = 0; i < 1000; i++) {
		struct map_entry e = map_select(&m, i);

		assert(memcmp(&i, e.value, sizeof(int)) == 0);

		assert(map_rank(&m, e.key, e.keylen) == (size_t) i);
	}

	map_destroy(&m);

	return EXIT_SUCCESS;
}
