/**
 * @file hmap.h
 * @brief Basic hashmap.
 *
 * ```
 * #define T <key-type>, <value-type>, <name>[, (<flags>)]
 *
 * // Function to override hash function input.
 * #define fmap_key <map-key-fn>
 *
 * // Function takes key type as input and returns its hash.
 * #define fuse_hash <hash-fn>
 *
 * // map_key function should have the signature:
 * void my_map_key(const key_type *key,
 *                 size_t key_len,
 *                 const char **out_bytes,
 *                 size_t *out_len)
 * {
 *     // default implementation
 *     *out_len = key_len;
 *     *out_bytes = &key;
 * }
 *
 * // hash function should have signature:
 * uint64_t my_hash(const char *key, size_t key_len)
 * {
 *     // default implementation is the FNV hash function.
 *     ...
 * }
 * ```
 *
 * This implementation uses open addressing method. It allows variable-length
 * keys while storing fixed-length values.
 */

#ifndef LFI_DOXYGEN

#ifndef LF_HASHMAP_H
#define LF_HASHMAP_H

#include "priv/detail.h"
#include "priv/template.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LF_HASHMAP_INITIAL_CAP 64
#define LFI_HASHMAP_TOMBSTONE ((void *) -1)

uint64_t lfi_g(fnv_hash)(const char *, size_t);

#endif  // LF_HASHMAP_H

#ifndef T
#define T char, int, a
#endif

#define lfi_ctype hmap
#define lfi_name lfi_arg3(T)
#define lfi_flags lfi_arg4(T, 0, 0, 0)

#include "priv/linkage.h"

#define lfi_key lfi_arg1(T)
#define lfi_value lfi_arg2(T)

#else  // LFI LFI_DOXYGEN

#include "priv/template.h"

#define lfi_self struct fhmap
#define lfi_key key_type
#define lfi_value value_type
#define lfi_memb(name) fhmap_ ## name

#endif


/** @brief hmap. */
lfi_self {
	/** @cond */
	struct lfi(entry) **lfi(entries);
	size_t lfi(cap);
	size_t lfi(used);
	/** @endcond */
};

/** @brief Key value pair. */
struct lfi_memb(entry_mut) {
	lfi_value *value  /** Value. */;
	const lfi_key *key  /** The key. */;
	size_t key_len  /** Length of the key. */;
};

/** @brief Key value pair. */
struct lfi_memb(entry) {
	lfi_value const *value  /** Value. */;
	const lfi_key *key  /** The key. */;
	size_t key_len  /** Length of the key. */;
};

/** @brief Iteration handle. */
struct lfi_memb(it) {
	/* @cond */
	const lfi_self *lfi(m);
	size_t lfi(i);
	/* @endcond */
};

/** @brief Mutable iteration handle. */
struct lfi_memb(it_mut) {
	/* @cond */
	lfi_self *lfi(m);
	size_t lfi(i);
	/* @endcond */
};


/* @cond */
struct lfi(entry) {
	lfi_value value;
	size_t key_len;
	lfi_key key[];
};

/* Cast internal entry to public entry type. */
static inline struct lfi_memb(entry) lfi(new_pub_entry)(struct lfi(entry) *e)
{
	return (struct lfi_memb(entry)) {
		.value = &e->value,
		.key = e->key,
		.key_len = e->key_len,
	};
}
/* Cast mutable entry to immuatble entry. */
static inline struct lfi_memb(entry_mut) lfi(as_mut)(struct lfi_memb(entry) *e)
{
	return (struct lfi_memb(entry_mut)) {
		.value = (lfi_value *) e->value,
		.key = e->key,
		.key_len = e->key_len,
	};
}

static inline uint64_t lfi(hash)(const lfi_key *key, size_t key_len)
{
	const char *mapped_key_bytes;
	size_t mapped_key_len;
	uint64_t hash;
#ifdef fmap_key
	fmap_key(key, key_len, &mapped_key_bytes, &mapped_key_len);
#else
	mapped_key_bytes = (const char *) key;
	mapped_key_len = key_len;
#endif
#ifdef fuse_hash
	hash = fuse_hash(mapped_key_bytes, mapped_key_len);
#else
	hash = lfi_g(fnv_hash)(mapped_key_bytes, mapped_key_len);
#endif
	return hash;
}

static inline struct lfi(entry) **lfi(get_entry)(lfi_self *m,
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
	if (m->lfi(cap) == new_cap)
		return 0;

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

	lfi_assert(lfi(get_entry)(m, key, key_len) == NULL,
			"hmap contains the element");

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


/** @brief Idetical to fhmap_init(), but accepts a capacity argument */
lfi_wur static inline int lfi_memb(with_cap)(lfi_self *m, size_t cap)
{
	if (cap == 0)
		cap = 1;

	m->lfi(cap) = cap;
	m->lfi(used) = 0;
	m->lfi(entries) = calloc(cap, sizeof(struct lfi(hmap_entry) *));

	return m->lfi(entries) == NULL ? 1 : 0;
}

/** @brief Identical to fhmap_init(), but raises an error if memory allocation
 * fails. */
static inline void lfi_memb(xwith_cap)(lfi_self *m, size_t cap)
{
	lfi_unwrap(lfi_memb(with_cap)(m, cap) == 0);
}

/** @brief Creates a new hashmap, returns non-zero if a memory allocation
 * failure occurs. */
lfi_wur static inline int lfi_memb(init)(lfi_self *m)
{
	return lfi_memb(with_cap)(m, LF_HASHMAP_INITIAL_CAP);
}

/** @brief Identical to fhmap_init(), but raises an error if memory allocation
 * fails. */
static inline void lfi_memb(xinit)(lfi_self *m)
{
	lfi_unwrap(lfi_memb(init)(m) == 0);
}

/** @brief Clears all of the memory allocated by the hashmap. */
static inline void lfi_memb(destroy)(lfi_self *m)
{
	for (size_t i = 0; i < m->lfi(cap); i++) {
		struct lfi(entry) *e = m->lfi(entries)[i];

		if (e != NULL && e != LFI_HASHMAP_TOMBSTONE)
			free(e);
	}

	free(m->lfi(entries));
}

/** @brief Returns true if a matching the key value pair foud, and out_entry
 * set to this pair. */
static inline bool lfi_memb(gete)(const lfi_self *m,
				  const lfi_key *key,
				  size_t key_len,
				  struct lfi_memb(entry) *out_entry)
{

	lfi_debug_assertion(key_len != 0, "key length cannot be zero");

	struct lfi(entry) **e = lfi(get_entry)((lfi_self *) m, key, key_len);

	if (e == NULL)
		return false;

	if (out_entry != NULL)
		*out_entry = lfi(new_pub_entry)(*e);

	return true;
}

/** @brief Identical to fhmap_gete(), but the key_len is sizeof(key_type). */
static inline bool lfi_memb(get2e)(const lfi_self *m,
				   const lfi_key *key,
				   struct lfi_memb(entry) *out_entry)
{ return lfi_memb(gete)(m, key, sizeof(lfi_key), out_entry); }

/** @brief Identical to fhmap_gete(), but accepts a null-terminated key. */
static inline bool lfi_memb(get3e)(const lfi_self *m,
				   const lfi_key key[],
				   struct lfi_memb(entry) *out_entry)
{ return lfi_memb(gete)(m, key, strlen((const char *) key) + 1, out_entry); }

#define lfi_define_get_fns(get_fn, params, args) \
	/** @brief Identical to @ref fhmap_ ## get_fn, but returns a mutable
	   entry. */ \
	static inline bool lfi_memb(get_fn ## e ## _mut) \
	(lfi_remove_paren(params), struct lfi_memb(entry_mut) *out_entry) \
	{ \
		struct lfi_memb(entry) out_entry_const; \
		bool res = lfi_memb(get_fn ## e)(lfi_remove_paren(args), \
					         &out_entry_const); \
		if (out_entry != NULL) \
			*out_entry = lfi(as_mut)(&out_entry_const); \
		return res; \
	} \
	/** @brief Identical to @ref fhmap_ ## get_fn ## e, but returns a
	   pointer to value. */ \
	static inline lfi_value const * \
	lfi_memb(get_fn)(const lfi_remove_paren(params)) \
	{ \
		struct lfi_memb(entry) e; \
		return lfi_memb(get_fn ## e)(lfi_remove_paren(args), &e) ? \
			e.value : NULL; \
	} \
	/** @brief Identical to @ref fhmap_ ## get_fn ## e_mut, but returns a
	   mutable pointer to value. */ \
	static inline lfi_value * \
	lfi_memb(get_fn ## _mut) params \
	{ \
		struct lfi_memb(entry_mut) e; \
		return lfi_memb(get_fn ## e_mut)(lfi_remove_paren(args), &e) ? \
			e.value : NULL; \
	}

lfi_define_get_fns(get, (lfi_self *m, const lfi_key *key, size_t key_len),
		   (m, key, key_len))
lfi_define_get_fns(get2, (lfi_self *m, const lfi_key *key), (m, key))
lfi_define_get_fns(get3, (lfi_self *m, const lfi_key *key), (m, key))

#undef lfi_define_get_fns

/** @brief Inserts a key-value pair into the hmap and sets `out_entry` to
 * inserted entry.
 *
 * Returns non-zero if memory allocation fail occurs.
 *
 * @warning The `key` must not already exist in the hmap. */
static inline int lfi_memb(inserte)(lfi_self *m,
				    const lfi_key *key,
				    size_t key_len,
				    lfi_value const *value,
				    struct lfi_memb(entry_mut) *out_entry)
{
	lfi_assert(key_len != 0, "key length cannot be zero");

	struct lfi(entry) *e = lfi(insert)(m, key, key_len, value);

	if (e == NULL)
		return 1;

	struct lfi_memb(entry) out_entry_const = lfi(new_pub_entry)(e);
	if (out_entry != NULL)
		*out_entry = lfi(as_mut)(&out_entry_const);

	return 0;
}

/** @brief Identical to fhmap_insert(), but the key_len is sizeof(key_type). */
static inline int lfi_memb(insert2e)(lfi_self *m,
				     const lfi_key *key,
				     lfi_value const *value,
				     struct lfi_memb(entry_mut) *out_entry)
{ return lfi_memb(inserte)(m, key, sizeof(lfi_key), value, out_entry); }

/** @brief Identical to fhmap_insert(), but accepts a null-terminated key. */
static inline int lfi_memb(insert3e)(lfi_self *m,
				     const lfi_key key[],
				     lfi_value const *value,
				     struct lfi_memb(entry_mut) *out_entry)
{ return lfi_memb(inserte)(m, key, strlen((const char *) key) + 1, value, out_entry); }

#define lfi_define_insert_fns(insert_fn, params, args) \
	/** @brief Identical to @ref fhmap_ ## insert_fn ## e, but raises an
	    error if memory allocation fails. */ \
	static inline void lfi_memb(x ## insert_fn ## e) \
	(lfi_remove_paren(params), struct lfi_memb(entry_mut) *out_entry) \
	{ \
		struct lfi_memb(entry_mut) e; \
		lfi_unwrap(lfi_memb(insert_fn ## e)(lfi_remove_paren(args), &e) == 0); \
		if (out_entry != NULL) \
			*out_entry = e; \
	} \
	/** @brief Identical to @ref fhmap_ ## insert_fn ## e, but returns a
	   pointer to the inserted value. */ \
	static inline lfi_value *lfi_memb(insert_fn) params \
	{ \
		struct lfi_memb(entry_mut) e; \
		if (lfi_memb(insert_fn ## e)(lfi_remove_paren(args), &e) == 0) \
			return e.value; \
		else \
			return NULL; \
	} \
	/** @brief Identical to @ref fhmap_ ## insert_fn, but raises an error
	    if memory allocation fails. */ \
	static inline lfi_value *lfi_memb(x ## insert_fn) params \
	{ \
		lfi_value *inserted_value = lfi_memb(insert_fn) args; \
		lfi_unwrap(inserted_value != NULL); \
		return inserted_value; \
	}

lfi_define_insert_fns(insert,
		     (lfi_self *m, const lfi_key *key,
		      size_t key_len, lfi_value const *value),
		     (m, key, key_len, value))

lfi_define_insert_fns(insert2,
		     (lfi_self *m, const lfi_key *key, lfi_value const *value),
		     (m, key, value))

lfi_define_insert_fns(insert3,
		     (lfi_self *m, const lfi_key *key, lfi_value const *value),
		     (m, key, value))

#undef lfi_define_insert_fns

/** @brief Removes the key-value pair from the hmap, sets `out_value` to
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

	struct lfi(entry) **e = lfi(get_entry)((lfi_self *) m, key, key_len);

	if (e == NULL)
		return 0;

	if (out_val)
		*out_val = (*e)->value;

	m->lfi(used)--;
	free(*e);
	*e = LFI_HASHMAP_TOMBSTONE;
	return 1;
}

/** @brief Identical to fhmap_remove(), but the key_len is sizeof(key_type). */
static inline int lfi_memb(remove2)(lfi_self *m,
				    const lfi_key *key,
				    lfi_value *out_val)
{
	return lfi_memb(remove)(m, key, sizeof(lfi_key), out_val);
}

/** @brief Identical to fhmap_remove(), but accepts a null-terminated key list. */
static inline int lfi_memb(remove3)(lfi_self *m,
				    const lfi_key key[],
				    lfi_value *out_val)
{
	return lfi_memb(remove)(m, key, strlen((const char *) key) + 1, out_val);
}


/** @brief Returns total number of the key-value pairs. */
static inline size_t lfi_memb(used)(const lfi_self *m)
{
	return m->lfi(used);
}

/** @brief Returns capacity of the hashmap. */
static inline size_t lfi_memb(cap)(const lfi_self *m)
{
	return m->lfi(cap);
}

/** @brief Clears entries of the hashmap. */
static inline void lfi_memb(clear)(lfi_self *m)
{
	for (size_t i = 0; i < m->lfi(cap); i++) {
		struct lfi(entry) **slot = &m->lfi(entries)[i];

		if (*slot != NULL) {
			if (*slot != LFI_HASHMAP_TOMBSTONE)
				free(*slot);

			*slot = NULL;
		}
	}

	m->lfi(used) = 0;
}

/** @brief Shrinks the map to given capacity.
 *
 * If a capacity lower than the number of stored elements is specified, the
 * capacity is reduced to the number of stored elements.
 *
 * See hmap_shrink_to_fit()
 *
 * Returns non-zero if a memory allocation failure occurs. */
static inline int lfi_memb(shrink_to)(lfi_self *m, size_t new_cap)
{
	if (new_cap < m->lfi(used))
		return lfi(resize)(m, m->lfi(used));
	else if (new_cap == 0)
		return lfi(resize)(m, 1);
	else
		return lfi(resize)(m, new_cap);
}

/** @brief Shrinks the hashmap to ~133% of its used element count, if its
 * capacity is larger than that.
 *
 * Use hmap_shrink_to(&m, 0) if you really want to shrink the capacity of the
 * map as much as possible.
 *
 * @note This operation is best-effort shrink, the capacity may left as-is
 *       if a memory allocation error occur.
 *
 * Returns non-zero if a memory allocation failure occurs. */
static inline int lfi_memb(shrink_to_fit)(lfi_self *m)
{
	return lfi_memb(shrink_to)(m, (m->lfi(used) + 1) * 4 / 3);
}

/** @brief Reserves capacity for at least `additional` more elements to be
 * inserted in the hashmap.
 *
 * The collection may reserve more space to speculatively avoid frequent
 * reallocations.
 *
 * @note This operation is best-effort grow, the capacity may left as-is if a
 *       memory allocation failure occur.
 *
 * Returns non-zero if a memory allocation failure occurs. */
static inline int lfi_memb(reserve)(lfi_self *m, size_t additional)
{
	if (additional == 0)
		return 0;

	size_t new_cap = (m->lfi(used) + additional) * 4 / 3;

	if (new_cap > m->lfi(cap))
		return lfi(resize)(m, new_cap);
	else
		return 0;
}

/** @brief Initializes a new iteration handle. */
static inline struct lfi_memb(it) lfi_memb(iter)(const lfi_self *m)
{
	return (struct lfi_memb(it)) { .lfi(m) = m, .lfi(i) = 0 };
}

/** @brief Initializes a new mutable iteration handle. */
static inline struct lfi_memb(it_mut) lfi_memb(iter_mut)(lfi_self *m)
{
	return (struct lfi_memb(it_mut)) { .lfi(m) = m, .lfi(i) = 0 };
}

/** @brief See fhmap_iter_next() */
static inline bool lfi_memb(iter_next)(struct lfi_memb(it) *it,
				       struct lfi_memb(entry) *out)
{
	for (; it->lfi(i) < it->lfi(m)->lfi(cap); it->lfi(i)++) {
		struct lfi(entry) *e = it->lfi(m)->lfi(entries)[it->lfi(i)];

		if (e != NULL && e != LFI_HASHMAP_TOMBSTONE) {
			*out = lfi(new_pub_entry)(e);
			it->lfi(i)++;
			return true;
		}
	}

	return false;
}

/** @brief Advances the iterator and sets out to the next value.
 *
 * Returns true if out set to an entry. */
static inline bool lfi_memb(iter_next_mut)(struct lfi_memb(it_mut) *it,
				           struct lfi_memb(entry_mut) *out)
{
	struct lfi_memb(entry) e;
	struct lfi_memb(it) it_const = {
		.lfi(m) = (lfi_self *) it->lfi(m),
		.lfi(i) = it->lfi(i)
	};
	bool res = lfi_memb(iter_next)(&it_const, &e);

	it->lfi(i) = it_const.lfi(i);
	*out = lfi(as_mut)(&e);

	return res;
}


#undef fmap_key
#undef fuse_hash

/* @cond */
#include "priv/finalize.h"
/* @endcond */
