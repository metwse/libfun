#define LIBFUN_PREFIX t

#define T int, int, int
#include "../../include/hmap.h"

#define T char, int, str
#include "../../include/hmap.h"

#define T char, char *, str2str
#include "../../include/hmap.h"

#define T char, const char *, str2const_str
#include "../../include/hmap.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct my_str {
	size_t len;
	char *chars;
};

void my_map_key(const struct my_str *key,
                size_t key_len,
                const char **out_bytes,
                size_t *out_len)
{
	/* cannot map array of strings */
	assert(key_len == sizeof(struct my_str));

	*out_len = key->len;
	*out_bytes = key->chars;
}

uint64_t my_hash(const char *key, size_t key_len)
{
	return key_len + (key_len > 0 ? key[0] : 0);
}

#define T struct my_str, int, my_str
#define fmap_key my_map_key
#define fuse_hash my_hash

#include "../../include/hmap.h"


void test_basic(void)
{
	struct thmap_str2str m;
	thmap_str2str_xinit(&m);

	assert(strcmp(*thmap_str2str_xinsert3(&m, "key", &(char *) { "value" }),
		      "value") == 0);
	assert(strcmp(*thmap_str2str_get3(&m, "key"), "value") == 0);

	thmap_str2str_destroy(&m);

	struct thmap_str2const_str mc;
	thmap_str2const_str_xinit(&mc);

	const char *const_str = "test";
	thmap_str2const_str_xinsert3(&mc, "key", &const_str);

	const char *const_str2 = *thmap_str2const_str_get3(&mc, "key");
	/* still const, as the value type defined as const char * */
	const char *const_str3 = *thmap_str2const_str_get3_mut(&mc, "key");

	assert(strcmp(const_str, const_str2) == 0);
	assert(strcmp(const_str, const_str3) == 0);

	thmap_str2const_str_destroy(&mc);
}

void test_custom_hash(void)
{
	struct thmap_my_str m_my_str;
	thmap_my_str_xinit(&m_my_str);

	thmap_my_str_xinsert2(&m_my_str,
			      &(struct my_str) { .len = 4, .chars = "test", },
			      &(int) { 123 });

	thmap_my_str_xinsert2(&m_my_str,
			      &(struct my_str) { .len = 4, .chars = "tst2", },
			      &(int) { 321 });

	assert(*thmap_my_str_get2(&m_my_str,
				  &(struct my_str) {
					.len = 4,
					.chars = "tst2",
				  }) == 321);

	assert(*thmap_my_str_get2(&m_my_str,
				  &(struct my_str) {
					.len = 4,
					.chars = "test",
				  }) == 123);

	thmap_my_str_destroy(&m_my_str);
}

void test_fuzz(void)
{
	struct thmap_int m_int;
	struct thmap_str m_str;

	char buf[128];

	for (int _fuzz = 0; _fuzz < 32; _fuzz++) {
		thmap_int_xinit(&m_int);
		thmap_str_xinit(&m_str);

		int limit = rand() % 4096;
		int values[limit];

		for (int i = 0; i < limit; i++) {
			values[i] = rand();
			sprintf(buf, "%d", values[i]);

			// hashmap does not contain the key
			assert(!thmap_int_get2(&m_int, &i));
			assert(!thmap_str_get3(&m_str, buf));

			thmap_int_xinsert2(&m_int, &i, &values[i]);
			thmap_str_xinsert3(&m_str, buf, &values[i]);

			// now it contain
			assert(thmap_int_get2(&m_int, &i));
			assert(thmap_str_get3(&m_str, buf));

			assert(*thmap_int_get2(&m_int, &i) == values[i]);
			assert(*thmap_str_get3(&m_str, buf) == values[i]);
		}


		int limit2 = limit % ((rand() % 2048) + 4);
		// remove all even-numbered keys
		for (int i = 0; i < limit2; i += 2) {
			sprintf(buf, "%d", values[i]);

			// check element
			assert(*thmap_int_get2(&m_int, &i) == values[i]);
			assert(*thmap_str_get3(&m_str, buf) == values[i]);

			// now remove it
			int remove_out_i;
			int remove_out_str;
			assert(thmap_int_remove2(&m_int, &i, &remove_out_i));
			assert(thmap_str_remove3(&m_str, buf, &remove_out_str));
			assert(remove_out_i == values[i] &&
			       remove_out_str == values[i]);

			// the key should not present
			assert(thmap_int_get2(&m_int, &i) == NULL);
			assert(thmap_str_get3(&m_str, buf) == NULL);
		}

		thmap_str_destroy(&m_str);
		thmap_int_destroy(&m_int);
	}
}


int main(void)
{
	test_basic();
	test_custom_hash();
	test_fuzz();
}
