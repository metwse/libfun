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
	thmap_str2str_xwith_cap(&m, 0);

	assert(strcmp(*thmap_str2str_xinsert3(&m, "key", &(char *) { "value" }),
		      "value") == 0);

	struct thmap_str2str_entry_mut inserted_e;
	thmap_str2str_xinsert3e(&m, "key2", &(char *) { "value2-initial" },
				&inserted_e);

	assert(strcmp(*thmap_str2str_get3(&m, "key"), "value") == 0);
	assert(strcmp(*inserted_e.value, "value2-initial") == 0);
	*inserted_e.value = "value2";

	struct thmap_str2str_entry e;  /* type assertions */
	struct thmap_str2str_entry_mut e_mut;
	assert(thmap_str2str_get3e(&m, "key2", &e));
	assert(thmap_str2str_get3e_mut(&m, "key2", &e_mut));
	assert(strcmp(*e_mut.value, "value2") == 0);
	*e_mut.value = "value2-changed";
	assert(strcmp(*e.value, "value2-changed") == 0);

	char *removed;
	assert(thmap_str2str_remove3(&m, "key", &removed));
	assert(thmap_str2str_remove3(&m, "key", &removed) == 0);  /* already removed */
	assert(strcmp(removed, "value") == 0);

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
			      &(struct my_str) { .len = 4, .chars = "test" },
			      &(int) { 123 });

	thmap_my_str_xinsert2(&m_my_str,
			      &(struct my_str) { .len = 4, .chars = "tst2" },
			      &(int) { 321 });

	assert(*thmap_my_str_get2(&m_my_str,
				  &(struct my_str) {
					.len = 4,
					.chars = "tst2"
				  }) == 321);

	assert(*thmap_my_str_get2(&m_my_str,
				  &(struct my_str) {
					.len = 4,
					.chars = "test"
				  }) == 123);

	thmap_my_str_destroy(&m_my_str);
}

void test_iterators(void)
{
	struct thmap_int m;
	thmap_int_xinit(&m);

	int values[64];
	int values_sum = 0;

	for (int i = 0; i < 64; i++) {
		values_sum += (values[i] = rand());
		thmap_int_xinsert2(&m, &i, &values[i]);
	}

	bool reached_values[64] = { 0 };
	int reached_values_sum = 0;
	struct thmap_int_it it = thmap_int_iter(&m);
	struct thmap_int_entry e;
	while (thmap_int_iter_next(&it, &e)) {
		reached_values[*e.key] = true;
		reached_values_sum += *e.value;
		assert(values[*e.key] == *e.value);
	}

	/* ensure every key is reached */
	for (int i = 0; i < 64; i++)
		assert(reached_values[i]);
	assert(reached_values_sum == values_sum);

	/* set all key-value pairs to (key, key) */
	struct thmap_int_it_mut it_mut = thmap_int_iter_mut(&m);
	struct thmap_int_entry_mut e_mut;
	while (thmap_int_iter_next_mut(&it_mut, &e_mut))
		*e_mut.value = *e_mut.key;

	for (int i = 0; i < 64; i++)
		assert(*thmap_int_get2(&m, &i) == i);

	thmap_int_destroy(&m);
}

void test_fuzz(void)
{
	struct thmap_int m_int;
	struct thmap_str m_str;

	char buf[128];

	for (int _fuzz = 0; _fuzz < 32; _fuzz++) {
		thmap_int_xwith_cap(&m_int, 1);
		assert(thmap_int_shrink_to(&m_int, 0) == 0);

		thmap_str_xwith_cap(&m_str, 1);

		int limit = rand() % 4096 + 4;
		int values[limit];
		int values_sum = 0;

		/* noop */
		assert(thmap_int_reserve(&m_int, 0) == 0);
		/* we'll reserve capacity beforehand, and check there will not
		 * be any reallocation */
		assert(thmap_int_reserve(&m_int, limit) == 0);
		size_t m_int_cap = thmap_int_cap(&m_int);

		/* we already reserved a much bigger additional cap */
		assert(thmap_int_reserve(&m_int, 1) == 0);

		for (int i = 0; i < limit; i++) {
			values_sum += (values[i] = rand());
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

		struct thmap_int_it it = thmap_int_iter(&m_int);
		struct thmap_int_entry m_int_e;
		int values_int_sum = 0;
		while (thmap_int_iter_next(&it, &m_int_e)) {
			const int *key = m_int_e.key; /* type assertions */
			const int *value = m_int_e.value;
			assert(*thmap_int_get2(&m_int, key) == *value);
			assert(*thmap_int_get2(&m_int, key) == values[*key]);
			values_int_sum += *value;
		}
		assert(values_int_sum == values_sum);

		struct thmap_str_it_mut it_mut = thmap_str_iter_mut(&m_str);
		struct thmap_str_entry_mut m_str_e;
		int values_str_sum = 0;
		while (thmap_str_iter_next_mut(&it_mut, &m_str_e)) {
			const char *key = m_str_e.key; /* type assertions */
			int *value = m_str_e.value; /* is mutable */
			assert(*thmap_str_get3(&m_str, key) == *value);
			values_str_sum += *value;
		}
		assert(values_str_sum == values_sum);

		/* no reallocation should occur as we have reserved cap */
		assert(thmap_int_cap(&m_int) == m_int_cap);

		assert(thmap_int_shrink_to_fit(&m_int) == 0);
		assert(thmap_int_used(&m_int) == (size_t) limit);

		/* shrink as much as possible */
		assert(thmap_str_shrink_to(&m_str, 0) == 0);
		/* noop */
		assert(thmap_str_shrink_to(&m_str, thmap_str_cap(&m_str)) == 0);
		assert(thmap_str_cap(&m_str) == (size_t) limit);

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

		thmap_int_clear(&m_int);
		assert(thmap_int_shrink_to_fit(&m_int) == 0);
		for (int i = 1; i < limit2; i += 2) {
			assert(thmap_int_get2(&m_int, &i) == NULL);
			thmap_int_xinsert2(&m_int, &i, &values[i]);
		}

		thmap_str_destroy(&m_str);
		thmap_int_destroy(&m_int);
	}
}


int main(void)
{
	test_basic();
	test_custom_hash();
	test_iterators();
	test_fuzz();
}
