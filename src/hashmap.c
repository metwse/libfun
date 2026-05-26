#ifndef LF_HEADERONLY
#include "common.h"
#include "../include/config.h"
#include "../include/hashmap.h"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define LF_HASHMAP_TOMBSTONE SIZE_MAX

#define lf_hashmap_entry_at(entries, i) \
	((struct lf(hashmap_entry) *) &((char *) entries) \
	 [i * (sizeof(struct lf(hashmap_entry)) + m->value_size)])


/* https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function */
lfi_fdecl(uint64_t, hashmap_fnv_hash)(const char *, size_t);

/* Normally, insert copies the key. But for rehashing, this function is
 * used. */
lfi_fdecl(int, hashmap_insert2_nocopy)(struct lf(hashmap) *,
				       const void *,
				       size_t, const void *);

/* Rehash the hashmap to a bigger hash bucket. */
lfi_fdecl(int, hashmap_rehash)(struct lf(hashmap) *, size_t);

/* Get an entry, key-value pair. */
lfi_fdecl(struct lf(hashmap_entry) *, hashmap_get2_entry)(struct lf(hashmap) *,
							  const void *,
							  size_t);


lfi_fdecl(uint64_t, hashmap_fnv_hash)(const char *key, size_t keylen)
{
	uint64_t hash = 0xcbf29ce484222325;

	for (size_t i = 0; i < keylen; i++) {
		hash *= 0x100000001b3;
		hash ^= (unsigned char) key[i];
	}

	return hash;
}


int lf(hashmap_init)(struct lf(hashmap) *m, size_t value_size)
{
	m->cap = LF_HASHMAP_INITIAL_CAP;
	m->used = 0;
	/* Align value size to sizeof(size_t)-byte boundary */
	m->value_size =
		((value_size + sizeof(size_t) - 1) / sizeof(size_t)) *
			sizeof(size_t);

	m->entries = calloc(LF_HASHMAP_INITIAL_CAP,
			    m->value_size + sizeof(struct lf(hashmap_entry)));

	return m->entries == NULL ? 1 : 0;

}

void lf(hashmap_destroy)(struct lf(hashmap) *m)
{
	for (size_t i = 0; i < m->cap; i++) {
		struct lf(hashmap_entry) *e = lf_hashmap_entry_at(m->entries, i);

		if (e->keylen != LF_HASHMAP_TOMBSTONE && e->keylen != 0)
			free((void *) e->key);
	}

	free(m->entries);
}

void *lf(hashmap_get)(struct lf(hashmap) *m, const void *key)
{
	return lf(hashmap_get2)(m, key, strlen(key));
}

void *lf(hashmap_get2)(struct lf(hashmap) *m, const void *key, size_t keylen)
{
	struct lf(hashmap_entry) *e = lfi(hashmap_get2_entry)(m, key, keylen);

	return e ? e->value : NULL;
}

void *lf(hashmap_remove)(struct lf(hashmap) *m, const void *key)
{
	return lf(hashmap_remove2)(m, key, strlen(key));
}

void *lf(hashmap_remove2)(struct lf(hashmap) *m, const void *key, size_t keylen)
{
	struct lf(hashmap_entry) *e = lfi(hashmap_get2_entry)(m, key, keylen);

	if (e) {
		e->keylen = LF_HASHMAP_TOMBSTONE;
		free((void *) e->key);
		m->used--;

		return e->value;
	} else {
		return NULL;
	}
}

int lf(hashmap_insert)(struct lf(hashmap) *m, const void *key, const void *value)
{
	return lf(hashmap_insert2)(m, key, strlen(key), value);
}

int lf(hashmap_insert2)(struct lf(hashmap) *m,
			const void *key,
			size_t keylen,
			const void *value)
{
	void *new_key = malloc(keylen);

	if (new_key == NULL)
		return 1;

	memcpy(new_key, key, keylen);

	return lfi(hashmap_insert2_nocopy)(m, new_key, keylen, value);
}

void lf(hashmap_iter)(struct lf(hashmap) *m, struct lf(hashmap_it) *it)
{
	it->m = m;
	it->i = 0;
}

struct lf(hashmap_entry) *lf(hashmap_iter_next)(struct lf(hashmap_it) *it)
{
	struct lf(hashmap) *m = it->m;

	while (it->i != m->cap) {
		struct lf(hashmap_entry) *e =
			lf_hashmap_entry_at(m->entries, it->i);

		it->i++;

		if (e->keylen != LF_HASHMAP_TOMBSTONE && e->keylen != 0)
			return e;
	}

	return NULL;
}

void lf(hashmap_xinit)(struct lf(hashmap) *m, size_t value_size)
{
	lf_unwrap(lf(hashmap_init)(m, value_size));
}

void lf(hashmap_xinsert)(struct lf(hashmap) *m,
			 const void *key,
			 const void *value)
{
	lf_unwrap(lf(hashmap_insert)(m, key, value));
}

void lf(hashmap_xinsert2)(struct lf(hashmap) *m,
			  const void *key,
			  size_t keylen,
			  const void *value)
{
	lf_unwrap(lf(hashmap_insert2)(m, key, keylen, value));
}

lfi_fdecl(struct lf(hashmap_entry) *, hashmap_get2_entry)(struct lf(hashmap) *m,
							  const void *key,
							  size_t keylen)
{
	uint64_t hash = lfi(hashmap_fnv_hash)(key, keylen);
	size_t start_i = hash % m->cap;
	size_t i = start_i;

	do {
		struct lf(hashmap_entry) *e = lf_hashmap_entry_at(m->entries, i);

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

lfi_fdecl(int, hashmap_rehash)(struct lf(hashmap) *m, size_t old_cap)
{
	struct lf(hashmap_entry) *old_entries = m->entries;

	m->used = 0;
	m->entries = calloc(m->cap,
			    m->value_size + sizeof(struct lf(hashmap_entry)));

	if (m->entries == NULL)
		return 1;

	for (size_t i = 0; i < old_cap; i++) {
		struct lf(hashmap_entry) *e = lf_hashmap_entry_at(old_entries, i);

		if (e->keylen != LF_HASHMAP_TOMBSTONE && e->keylen != 0)
			lfi(hashmap_insert2_nocopy)(m, e->key, e->keylen, e->value);
	}

	free(old_entries);

	return 0;
}

lfi_fdecl(int, hashmap_insert2_nocopy)(struct lf(hashmap) *m,
				       const void *key,
				       size_t keylen,
				       const void *value)
{
	if (m->cap * 3 < m->used * 4) {
		size_t old_cap = m->cap;

		m->cap *= 2;

		if (lfi(hashmap_rehash)(m, old_cap))
			return 1;
	}

	lf_assert(!lf(hashmap_get2)(m, key, keylen),
		  "hashmap contains the element");

	uint64_t hash = lfi(hashmap_fnv_hash)(key, keylen);
	size_t start_i = hash % m->cap;
	size_t i = start_i;

	do {
		struct lf(hashmap_entry) *e = lf_hashmap_entry_at(m->entries, i);

		if (e->keylen == 0 || e->keylen == LF_HASHMAP_TOMBSTONE) {
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

	lf_unreachable;  // GCOVR_EXCL_LINE: unreachable
}
