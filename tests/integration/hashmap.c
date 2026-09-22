#define T int, int, int
#include "../../include/hashmap.h"

#define T char, int, str
#include "../../include/hashmap.h"

#define T char, char *, str2str
#include "../../include/hashmap.h"

#define T char, const char *, str2const_str
#include "../../include/hashmap.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


int main(void)
{
	struct lf(hashmap_str2str) m;
	lf(hashmap_str2str_xinit)(&m);
	assert(strcmp(*lf(hashmap_str2str_xinsert3)(&m,
						   "key",
						   &(char *) { "value" }),
			  "value") == 0);

	char *stored_value = *lf(hashmap_str2str_get3)(&m, "key");
	assert(strcmp(stored_value, "value") == 0);
	lf(hashmap_str2str_destroy)(&m);

	struct lf(hashmap_str2const_str) mc;
	lf(hashmap_str2const_str_xinit)(&mc);
	const char *const_str = "test";

	lf(hashmap_str2const_str_xinsert3)(&mc, "key", &const_str);

	/* still const , as lfi_type defined as const char * */
	const char *const_str2 =
		*lf(hashmap_str2const_str_get3)(&mc, "key");
	const char *const_str3 =
		*lf(hashmap_str2const_str_get3_mut)(&mc, "key");  /* unsafe */
	assert(strcmp(const_str, const_str2) == 0);
	assert(strcmp(const_str, const_str3) == 0);

	lf(hashmap_str2const_str_destroy)(&mc);

	struct lf(hashmap_int) m_int;
	struct lf(hashmap_str) m_str;

	char buf[128];

	for (int _fuzz = 0; _fuzz < 128; _fuzz++) {
		lf(hashmap_int_xinit)(&m_int);
		lf(hashmap_str_xinit)(&m_str);

		int limit = rand() % 4096;
		int values[limit];

		for (int i = 0; i < limit; i++) {
			values[i] = rand();
			sprintf(buf, "%d", values[i]);

			// hashmap does not contain the key
			assert(!lf(hashmap_int_get2)(&m_int, &i));
			assert(!lf(hashmap_str_get3)(&m_str, buf));

			lf(hashmap_int_xinsert2)(&m_int, &i, &values[i]);
			lf(hashmap_str_xinsert3)(&m_str, buf, &values[i]);

			// now it contain
			assert(lf(hashmap_int_get2)(&m_int, &i));
			assert(lf(hashmap_str_get3)(&m_str, buf));

			assert(*lf(hashmap_int_get2)(&m_int, &i) == values[i]);
			assert(*lf(hashmap_str_get3)(&m_str, buf) == values[i]);
		}


		int limit2 = limit % ((rand() % 2048) + 4);
		// remove all even-numbered keys
		for (int i = 0; i < limit2; i += 2) {
			sprintf(buf, "%d", values[i]);

			// check element
			assert(*lf(hashmap_int_get2)(&m_int, &i) == values[i]);
			assert(*lf(hashmap_str_get3)(&m_str, buf) == values[i]);

			// now remove it
			int remove_out_i;
			int remove_out_str;
			assert(lf(hashmap_int_remove2)(&m_int, &i, &remove_out_i));
			assert(lf(hashmap_str_remove3)(&m_str, buf, &remove_out_str));
			assert(remove_out_i == values[i] &&
			       remove_out_str == values[i]);

			// the key should not present
			assert(lf(hashmap_int_get2)(&m_int, &i) == NULL);
			assert(lf(hashmap_str_get3)(&m_str, buf) == NULL);
		}

		lf(hashmap_str_destroy)(&m_str);
		lf(hashmap_int_destroy)(&m_int);
	}
}
