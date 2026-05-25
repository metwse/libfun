#include "../include/map.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define LF_MAP_TOMBSTONE SIZE_MAX

#define lf_map_entry_at(entries, i) ((struct lf(map_entry) *) &((char *) entries) \
			[i * (sizeof(struct lf(map_entry)) + m->value_size)])



/* https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function */
lfi_fdecl(uint64_t, map_fnv_hash)(const char *, size_t);

/* Normally, insert copies the key. But for rehashing, this function is
 * used. */
lfi_fdecl(int, map_insert2_nocopy)(struct lf(map) *,
				   const void *,
				   size_t, const void *);

/* Rehash the map to a bigger hash bucket. */
lfi_fdecl(int, map_rehash)(struct lf(map) *, size_t);

/* Get an entry, key-value pair. */
lfi_fdecl(struct lf(map_entry) *, map_get2_entry)(struct lf(map) *,
						  const void *,
						  size_t);


lfi_fdecl(uint64_t, map_fnv_hash)(const char *key, size_t keylen)
{
	uint64_t hash = 0xcbf29ce484222325;

	for (size_t i = 0; i < keylen; i++) {
		hash *= 0x100000001b3;
		hash ^= (unsigned char) key[i];
	}

	return hash;
}

int lf(map_init)(struct lf(map) *m, size_t value_size)
{
	m->cap = LF_MAP_INITIAL_CAP;
	m->used = 0;
	/* Align value size to sizeof(size_t)-byte boundary */
	m->value_size =
		((value_size + sizeof(size_t) - 1) / sizeof(size_t)) *
			sizeof(size_t);

	m->entries = calloc(LF_MAP_INITIAL_CAP,
			    m->value_size + sizeof(struct lf(map_entry)));

	return m->entries == NULL ? 1 : 0;

}

void lf(map_destroy)(struct lf(map) *m)
{
	for (size_t i = 0; i < m->cap; i++) {
		struct lf(map_entry) *e = lf_map_entry_at(m->entries, i);

		if (e->keylen != LF_MAP_TOMBSTONE && e->keylen != 0)
			free((void *) e->key);
	}

	free(m->entries);
}

void *lf(map_get)(struct lf(map) *m, const void *key)
{
	return lf(map_get2)(m, key, strlen(key));
}

void *lf(map_get2)(struct lf(map) *m, const void *key, size_t keylen)
{
	struct lf(map_entry) *e = lfif(map_get2_entry)(m, key, keylen);

	return e ? e->value : NULL;
}

void *lf(map_remove)(struct lf(map) *m, const void *key)
{
	return lf(map_remove2)(m, key, strlen(key));
}

void *lf(map_remove2)(struct lf(map) *m, const void *key, size_t keylen)
{
	struct lf(map_entry) *e = lfif(map_get2_entry)(m, key, keylen);

	if (e) {
		e->keylen = LF_MAP_TOMBSTONE;
		free((void *) e->key);
		m->used--;

		return e->value;
	} else {
		return NULL;
	}
}

int lf(map_insert)(struct lf(map) *m, const void *key, const void *value)
{
	return lf(map_insert2)(m, key, strlen(key), value);
}

int lf(map_insert2)(struct lf(map) *m, const void *key, size_t keylen, const void *value)
{
	void *new_key = malloc(keylen);

	if (new_key == NULL)
		return 1;

	memcpy(new_key, key, keylen);

	return lfif(map_insert2_nocopy)(m, new_key, keylen, value);
}

void lf(map_iter)(struct lf(map) *map, struct lf(map_it) *it)
{
	it->m = map;
	it->i = 0;
}

struct lf(map_entry) *lf(map_iter_next)(struct lf(map_it) *it)
{
	struct lf(map) *m = it->m;

	while (it->i != m->cap) {
		struct lf(map_entry) *e = lf_map_entry_at(m->entries, it->i);

		it->i++;

		if (e->keylen != LF_MAP_TOMBSTONE && e->keylen != 0)
			return e;
	}

	return NULL;
}

void lf(map_xinit)(struct lf(map) *m, size_t value_size)
{
	assert(lf(map_init)(m, value_size) == 0);
}

void lf(map_xinsert)(struct lf(map) *m,
		 const void *key,
		 const void *value)
{
	assert(lf(map_insert)(m, key, value) == 0);
}

void lf(map_xinsert2)(struct lf(map) *m,
		 const void *key,
		 size_t keylen,
		 const void *value)
{
	assert(lf(map_insert2)(m, key, keylen, value) == 0);
}

lfi_fdecl(struct lf(map_entry) *, map_get2_entry)(struct lf(map) *m,
					      const void *key,
					      size_t keylen)
{
	uint64_t hash = lfif(map_fnv_hash)(key, keylen);
	size_t start_i = hash % m->cap;
	size_t i = start_i;

	do {
		struct lf(map_entry) *e = lf_map_entry_at(m->entries, i);

		if (e->keylen == 0)
			return NULL;
		else if (e->keylen == keylen && memcmp(e->key, key, keylen) == 0)
			return e;
		else
			i++;

		i %= m->cap;
	} while (i != start_i);

	/* Hashmap filled up with tombstones */
	return NULL;
}

lfi_fdecl(int, map_rehash)(struct lf(map) *m, size_t cap)
{
	struct lf(map_entry) *old_entries = m->entries;

	m->used = 0;
	m->entries = calloc(m->cap,
			    m->value_size + sizeof(struct lf(map_entry)));

	if (m->entries == NULL)
		return 1;

	for (size_t i = 0; i < cap; i++) {
		struct lf(map_entry) *e = lf_map_entry_at(old_entries, i);

		if (e->keylen != LF_MAP_TOMBSTONE && e->keylen != 0)
			lfif(map_insert2_nocopy)(m, e->key, e->keylen, e->value);
	}

	free(old_entries);

	return 0;
}

lfi_fdecl(int, map_insert2_nocopy)(struct lf(map) *m,
				   const void *key,
				   size_t keylen,
				   const void *value)
{
	if (m->cap * 3 < m->used * 4) {
		size_t old_cap = m->cap;

		m->cap *= 2;

		if (lfif(map_rehash)(m, old_cap))
			return 1;
	}

	assert(!lf(map_get2)(m, key, keylen) && "map contains the element");

	uint64_t hash = lfif(map_fnv_hash)(key, keylen);
	size_t start_i = hash % m->cap;
	size_t i = start_i;

	do {
		struct lf(map_entry) *e = lf_map_entry_at(m->entries, i);

		if (e->keylen == 0 || e->keylen == LF_MAP_TOMBSTONE) {
			e->keylen = keylen;
			e->key = key;

			if (m->value_size)
				memcpy(e->value, value, m->value_size);

			m->used++;

			return 0;
		} else {
			i++;
		}

		i %= m->cap;
	} while (i != start_i);

	assert(0);  // GCOVR_EXCL_LINE: unreachable
}
