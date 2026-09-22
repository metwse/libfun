/**
 * @file hashmap.h
 * @brief Basic hashmap.
 *
 * ```
 * #define T <key-type>, <value-type>, <name>[, (<flags>)]
 *
 * #define fmap_key <key-fn> Function takes key type as input and returns
 *                           (char *, len).
 *
 * #define fuse_hash <hash-fn> Function takes key type as input and returns
 *                             its hash.
 *
 * // map_key function should have signature:
 * void my_map_key(const key_type *key,
 *                 size_t key_len,
 *                 const char **out_bytes,
 *                 size_t *out_len)
 * {
 *     // default implementation
 *     *out_len = key_len;
 *     **out_bytes = &key;
 * }
 *
 * // hash function should have signature:
 * uint64_t my_hash(const char *key, size_t key_len)
 * {
 *     // default implementation is the FNV hash function.
 * }
 * ```
 *
 * hashmap uses runs in open addressing method. It stores data as key-value
 * pairs and allows variable-length keys while storing fixed-length values.
 */

#include <stdint.h>
#ifndef LFI_DOXYGEN

/** @cond */
#ifndef LF_HASHMAP_H
#define LF_HASHMAP_H

#include "priv/detail.h"
#include "priv/template.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define LFI_HASHMAP_INITIAL_CAP 64
#define LFI_HASHMAP_TOMBSTONE ((void *) -1)

uint64_t lfi_g(fnv_hash)(const char *, size_t);

#endif  // LF_HASHMAP_H

#ifndef T
#define T char, int, a
#endif

#define lfi_ctype hashmap
#define lfi_name lfi_arg3(T)
#define lfi_flags lfi_arg4(T, 0, 0, 0)

#include "priv/linkage.h"

#define lfi_key lfi_arg1(T)
#define lfi_value lfi_arg2(T)

/** @endcond */
#else  // LFI LFI_DOXYGEN

#define lfi_self struct fhashmap
#define lfi_key key_type
#define lfi_value value_type
#define lfi_memb(name) fhashmap_ ## name

#endif


/** @brief hashmap. */
lfi_self {
	/** @cond */
	struct lfi(entry) **lfi(entries);
	size_t lfi(cap);
	size_t lfi(used);
	/** @endcond */
};

/** @brief Key value pair. */
struct lfi_memb(entry_mut) {
	const lfi_key *key  /** The key. */;
	size_t key_len  /** Length of the key. */;
	lfi_value *value  /** Value. */;
};

/** @brief Key value pair. */
struct lfi_memb(entry) {
	const lfi_key *key  /** The key. */;
	size_t key_len  /** Length of the key. */;
	lfi_value const *value  /** Value. */;
};

/* @cond */
struct lfi(entry) {
	lfi_value value;
	size_t key_len;
	lfi_key key[];
};

uint64_t lfi(hash)(const lfi_key *key, size_t key_len)
{
	const void *mapped_key_bytes;
	size_t mapped_key_len;
	uint64_t hash;
#ifdef fmap_key
	fmap_key (key, key_len, &mapped_key_bytes, &mapped_key_len);
#else
	mapped_key_bytes = (const void *) key;
	mapped_key_len = key_len;
#endif
#ifdef fuse_hash
	hash = fuse_hash((const char *) key, key_len);
#else
	hash = lfi_g(fnv_hash)(mapped_key_bytes, mapped_key_len);
#endif
	return hash;
}

static inline struct lfi(entry) **lfi(get_slot)(lfi_self *m,
						const lfi_key *key,
						size_t key_len)
{
	uint64_t hash = lfi(hash)(key, key_len);

	size_t start_i = hash % m->lfi(cap);
	size_t i = start_i;

	do {
		struct lfi(entry) **slot = &m->lfi(entries)[i];

		if (*slot == NULL)
			return NULL;
		else if (*slot != LFI_HASHMAP_TOMBSTONE &&
			 (*slot)->key_len == key_len &&
			 memcmp((*slot)->key, key, key_len) == 0)
			return slot;
		else
			i++;

		i %= m->lfi(cap);
	} while (i != start_i);

	/* Hashmap filled up with tombstones */
	return NULL;
}

static inline void lfi(insert_entry)(lfi_self *m, struct lfi(entry) *e)
{
	lfi_assert(m->lfi(cap) > m->lfi(used),
		   "call to insert_entry on full map");

	uint64_t hash = lfi(hash)(e->key, e->key_len);
	size_t start_i = hash % m->lfi(cap);
	size_t i = start_i;

	do {
		struct lfi(entry) **slot = &m->lfi(entries)[i];

		if (*slot == NULL || *slot == LFI_HASHMAP_TOMBSTONE) {
			m->lfi(used)++;
			*slot = e;

			return;
		} else {
			i++;
		}

		i %= m->lfi(cap);
	} while (i != start_i);

	lfi_unreachable;  // GCOVR_EXCL_LINE: unreachable
}

static inline int lfi(resize)(lfi_self *m, size_t new_cap)
{
	struct lfi(entry) **old_entries = m->lfi(entries);

	m->lfi(entries) = calloc(new_cap, sizeof(struct lfi(entry) *));

	if (m->lfi(entries) == NULL) {
		m->lfi(entries) = old_entries;
		return 1;
	}

	size_t old_cap = m->lfi(cap);
	m->lfi(used) = 0;
	m->lfi(cap) = new_cap;

	for (size_t i = 0; i < old_cap; i++) {
		struct lfi(entry) *e = old_entries[i];

		if (e != NULL)
			lfi(insert_entry)(m, e);
	}

	free(old_entries);
	return 0;
}

static inline struct lfi(entry) *lfi(insert)(lfi_self *m,
					     const lfi_key *key,
					     size_t key_len,
					     lfi_value const *value)
{
	if (m->lfi(cap) * 3 < m->lfi(used) * 4) {
		size_t new_cap = m->lfi(cap) * 2;

		lfi(resize)(m, new_cap);
	}

	if (m->lfi(cap) == m->lfi(used))
		return NULL;

	lfi_debug_assertion(!*lfi(get_slot)(m, key, key_len),
			"hashmap contains the element");

	struct lfi(entry) *e = (struct lfi(entry) *)
		malloc(sizeof(struct lfi(entry)) + key_len);

	e->key_len = key_len;
	memcpy(e->key, key, key_len);

	if (value != NULL)
		e->value = *value;

	 lfi(insert_entry)(m, e);

	 return e;
}
/* @endcond */


/** @brief Creates a new hashmap, returns non-zero if a memory allocation
 * failure occurs. */
lfi_wur static inline int lfi_memb(init)(lfi_self *m)
{
	m->lfi(cap) = LFI_HASHMAP_INITIAL_CAP;
	m->lfi(used) = 0;
	m->lfi(entries) = calloc(LFI_HASHMAP_INITIAL_CAP,
				 sizeof(struct lfi(hashmap_entry *)));

	return m->lfi(entries) == NULL ? 1 : 0;
}

/** @brief Identical to fhashmap_init(), but raises an error if memory allocation
 * fails. */
static inline void lfi_memb(xinit)(lfi_self *m)
{
	lfi_unwrap(lfi_memb(init)(m) == 0);
}

/** @brief Clears the memory allocated by the fhashmap. */
static inline void lfi_memb(destroy)(lfi_self *m)
{
	for (size_t i = 0; i < m->lfi(cap); i++) {
		struct lfi(entry) *e = m->lfi(entries)[i];

		if (e != NULL && e != LFI_HASHMAP_TOMBSTONE)
			free(e);
	}

	free(m->lfi(entries));
}

/** @brief Returns a pointer to the value matching the key, returns `NULL` if
 * the key is not found. */
static inline lfi_value const *lfi_memb(get)(const lfi_self *m,
					     const lfi_key *key,
					     size_t key_len)
{

	lfi_debug_assertion(key_len != 0,
			    "key length cannot be zero");

	struct lfi(entry) **slot = lfi(get_slot)((lfi_self *) m, key, key_len);

	if (slot == NULL || *slot == LFI_HASHMAP_TOMBSTONE)
		return NULL;

	return (lfi_value const *) &(*slot)->value;
}

/** @brief Identical to fhashmap_get(), but the key_len is sizeof(key_type). */
static inline lfi_value const *lfi_memb(get2)(const lfi_self *m,
					      const lfi_key *key)
{
	return lfi_memb(get)(m, key, sizeof(lfi_key));
}

/** @brief Identical to fhashmap_get(), but accepts a null-terminated key list. */
static inline lfi_value const *lfi_memb(get3)(const lfi_self *m,
					      const lfi_key key[])
{
	return lfi_memb(get)(m, key, strlen((const char *) key));
}

/** @brief Returns a pointer to the value matching the key, returns `NULL` if
 * the key is not found. */
static inline lfi_value *lfi_memb(get_mut)(lfi_self *m,
					   const lfi_key *key,
					   size_t key_len)
{
	return (lfi_value *) lfi_memb(get)(m, key, key_len);
}

/** @brief Identical to fhashmap_get(), but the key_len is sizeof(key_type). */
static inline lfi_value *lfi_memb(get2_mut)(lfi_self *m,
					    const lfi_key *key)
{
	return (lfi_value *) lfi_memb(get2)(m, key);
}

/** @brief Identical to fhashmap_get(), but accepts a null-terminated key list. */
static inline lfi_value *lfi_memb(get3_mut)(lfi_self *m,
					    const lfi_key key[])
{
	return (lfi_value *) lfi_memb(get3)(m, key);
}

/**
 * @brief Inserts a key-value pair into the hashmap.
 *
 * @warning The `key` must not already exist in the hashmap.
 */
static inline lfi_value *lfi_memb(insert)(lfi_self *m,
					  const lfi_key *key,
					  size_t key_len,
					  lfi_value const *value)
{
	lfi_debug_assertion(key_len != 0,
			    "key length cannot be zero");

	struct lfi(entry) *e = lfi(insert)(m, key, key_len, value);

	if (e == NULL)
		return NULL;

	return &e->value;
}

/** @brief Identical to fhashmap_insert(), but the key_len is sizeof(key_type). */
static inline lfi_value *lfi_memb(insert2)(lfi_self *m,
					   const lfi_key *key,
					   lfi_value const *value)
{
	return lfi_memb(insert)(m, key, sizeof(lfi_key), value);
}

/** @brief Identical to fhashmap_insert(), but the key_len is sizeof(key_type). */
static inline lfi_value *lfi_memb(insert3)(lfi_self *m,
					  const lfi_key key[],
					  lfi_value const *value)
{
	return lfi_memb(insert)(m, key, strlen((const char *) key), value);
}

/** @brief Idetical to fhashmap_insert(), but raises an error if memory
 * allocation fails. */
static inline lfi_value *lfi_memb(xinsert)(lfi_self *m,
					   const lfi_key *key,
					   size_t key_len,
					   lfi_value const *value)
{
	lfi_value *inserted_item = lfi_memb(insert)(m, key, key_len, value);
	lfi_unwrap(inserted_item != NULL);
	return inserted_item;
}

/** @brief Idetical to fhashmap_insert2(), but raises an error if memory
 * allocation fails. */
static inline lfi_value *lfi_memb(xinsert2)(lfi_self *m,
					    const lfi_key *key,
					    lfi_value const *value)
{
	lfi_value *inserted_item = lfi_memb(insert2)(m, key, value);
	lfi_unwrap(inserted_item != NULL);
	return inserted_item;
}

/** @brief Idetical to fhashmap_insert3(), but raises an error if memory
 * allocation fails. */
static inline lfi_value *lfi_memb(xinsert3)(lfi_self *m,
					    const lfi_key key[],
					    lfi_value const *value)
{
	lfi_value *inserted_item = lfi_memb(insert3)(m, key, value);
	lfi_unwrap(inserted_item != NULL);
	return inserted_item;
}

/** @brief Removes the key-value pair from the hashmap, sets `out_value` to
 * removed key if exists.
 *
 * Returns true if a pair removed. */
static inline int lfi_memb(remove)(lfi_self *m,
				   const lfi_key *key,
				   size_t key_len,
				   lfi_value *out_val)
{

	lfi_debug_assertion(key_len != 0,
			    "key length cannot be zero");

	struct lfi(entry) **slot = lfi(get_slot)((lfi_self *) m, key, key_len);

	if (slot == NULL || *slot == LFI_HASHMAP_TOMBSTONE)
		return 0;

	if (out_val)
		*out_val = (*slot)->value;

	m->lfi(used)--;
	free(*slot);
	*slot = LFI_HASHMAP_TOMBSTONE;
	return 1;
}

/** @brief Identical to fhashmap_remove(), but the key_len is sizeof(key_type). */
static inline int lfi_memb(remove2)(lfi_self *m,
				    const lfi_key *key,
				    lfi_value *out_val)
{
	return lfi_memb(remove)(m, key, sizeof(lfi_key), out_val);
}

/** @brief Identical to fhashmap_remove(), but accepts a null-terminated key list. */
static inline int lfi_memb(remove3)(lfi_self *m,
				    const lfi_key key[],
				    lfi_value *out_val)
{
	return lfi_memb(remove)(m, key, strlen((const char *) key), out_val);
}


#undef fmap_key
#undef fuse_hash

#include "priv/finalize.h"
