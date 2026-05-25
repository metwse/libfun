/**
 * @file hashmap.h
 * @brief Basic hashmap.
 *
 * hashmap uses the FNV hash function and open addressing method. It stores
 * data as key-value pairs, allowing variable-length keys while storing
 * fixed-length values.
 */

#ifndef LF_HASHMAP_H
#define LF_HASHMAP_H

#ifndef LF_HEADERONLY
#include "config.h"
#endif

#include <assert.h>
#include <stddef.h>


/** @brief hashmap. */
struct lf(hashmap) {
	/** @cond */
	struct lf(hashmap_entry) *entries;
	size_t cap;
	size_t used;
	size_t value_size;
	/** @endcond */
};

/** @brief Iteration handle to retrieve hashmap entries one by one. */
struct lf(hashmap_it) {
	/** @cond */
	struct lf(hashmap) *m;
	size_t i;
	/** @endcond */
};

/** @brief Key-value pair stored in the hashmap, entry. */
struct lf(hashmap_entry) {
	/** Key, the unique identifier pointing to the value. */
	const void *key;
	/** Length of the data stored in the key. */
	size_t keylen;
	/** The value assigned to the key by the user. */
	char value[];
};


/**
 * @brief Creates a new hashmap.
 *
 * Allocates the necessary memory for the hashmap. The `value_size` parameter
 * specifies the maximum size of the `value`s the user will add.
 *
 * Returns non-zero if a memory allocation failure occurs.
 */
int lf(hashmap_init)(struct lf(hashmap) *hashmap, size_t value_size) lfi_wur;

/** @brief Identical to hashmap_init(), but raises an error if memory allocation
 * fails. */
void lf(hashmap_xinit)(struct lf(hashmap) *hashmap, size_t value_size);

/** @brief Clears the memory allocated by the hashmap. */
void lf(hashmap_destroy)(struct lf(hashmap) *hashmap);

/**
 * @brief Returns a pointer to the value matching the key, returns `NULL` if
 * the key is not found.
 *
 * The `key` parameter must be null-terminated.
 */
void *lf(hashmap_get)(struct lf(hashmap) *hashmap, const void *key);

/** @brief Identical to hashmap_get(), but accepts a non-null-terminated key. */
void *lf(hashmap_get2)(struct lf(hashmap) *hashmap,
		       const void *key,
		       size_t keylen);

/**
 * @brief Inserts a key-value pair into the hashmap.
 *
 * @warning The `key` must not already exist in the hashmap.
 *
 * The `key` parameter must be null-terminated. Returns non-zero if the memory
 * allocation failure occurs.
 */
int lf(hashmap_insert)(struct lf(hashmap) *hashmap,
		       const void *key,
		       const void *value) lfi_wur;

/** @brief Identical to hashmap_insert(), but raises an error if memory
 * allocation fails. */
void lf(hashmap_xinsert)(struct lf(hashmap) *hashmap,
			 const void *key,
			 const void *value);

/** @brief Identical to hashmap_insert(), but accepts a non-null-terminated
 * key. */
int lf(hashmap_insert2)(struct lf(hashmap) *hashmap,
			const void *key,
			size_t keylen,
			const void *value) lfi_wur;

/** @brief Identical to hashmap_insert2(), but raises an error if memory
 * allocation fails. */
void lf(hashmap_xinsert2)(struct lf(hashmap) *hashmap,
			  const void *key,
			  size_t keylen,
			  const void *value);

/**
 * @brief Removes the key-value pair from the hashmap, returns a pointer to the
 * value.
 *
 * @attention The returned value pointer is valid until the next hashmap
 * operation. If the user wants to continue using the value, they must copy
 * underlying data.
 *
 * The `key` parameter must be `null-terminated`.
 */
void *lf(hashmap_remove)(struct lf(hashmap) *hashmap, const void *key);

/** @brief Identical to hashmap_remove(), but accepts a non-null-terminated
 * key. */
void *lf(hashmap_remove2)(struct lf(hashmap) *hashmap,
			  const void *key,
			  size_t keylen);

/** @brief Creates an iteration handle to retrieve the entries in the hashmap
 * one by one. */
void lf(hashmap_iter)(struct lf(hashmap) *hashmap, struct lf(hashmap_it) *it);

/**
 * @brief Retrieves the next entry from the iteration handle.
 *
 * If all entries have been retrieved, it returns `NULL`.
 */
struct lf(hashmap_entry) *lf(hashmap_iter_next)(struct lf(hashmap_it) *it);


#endif
