/**
 * @file map.h
 * @brief Basic map.
 *
 * map is an order-statistics tree implemented augmenting Red-Black tree.
 */

#ifndef LF_MAP_H
#define LF_MAP_H

#ifndef LF_HEADERONLY
#include "config.h"
#endif

#include <stddef.h>


/** @brief map. */
struct lf(map) {
	/** @cond */
	struct lfi(map_node) *root;

	void *hold_value;

	size_t value_size;

	int (*cmp)(const void *, const void *, size_t, size_t);
	/** @endcond */
};

/** @brief Iteration handle to retrieve map entries one by one. */
struct lf(map_it) {
	/** @cond */
	struct lf(map) *m;
	struct lfi(map_node) *n;
	/** @endcond */
};

/** @brief Map entry. */
struct lf(map_entry) {
	/** Key, the unique identifier pointing to the value. */
	const void *key;
	/** Length of the data stored in the key. */
	size_t keylen;
	/** The value assigned to the key by the user. */
	const void *value;
};

/** @cond */
struct lfi(map_node) {
	size_t keylen;

	struct lfi(map_node) *p;
	struct lfi(map_node) *left;
	struct lfi(map_node) *right;

	/* Number of nodes in the subtree rooted at this node *including*
	 * this node. */
	size_t size;

	/* 1 if red, 0 if black. */
	char color;

	/* Key and value. */
	char kv[];
};
/** @endcond */


/**
 * @brief Creates a new map.
 *
 * Comparator is a function pointer used to compare keys. It should return an
 * integer less than, equal to, or greater than zero if key1 is found,
 * respectively, to be less than, to match, or be greater than key2. Defaults
 * to memcmp(key1, key2, min(keylen1, keylen2)) if `NULL`.
 *
 * Returns non-zero if a memory allocation failure occurs.
 */
int lf(map_init)(struct lf(map) *map,
		 size_t value_size,
		 int (*comparator)(const void *key1,
				   const void *key2,
				   size_t keylen1,
				   size_t keylen2)) lfi_wur;

/** @brief Identical to map_init(), but raises an error if memory allocation
 * fails. */
void lf(map_xinit)(struct lf(map) *map,
		   size_t value_size,
		   int (*comparator)(const void *key1,
				     const void *key2,
				     size_t keylen1,
				     size_t keylen2));

/** @brief Clears the memory allocated by the map. */
void lf(map_destroy)(struct lf(map) *map);

/**
 * @brief Returns a pointer to the value matching the key, returns `NULL` if
 * the key is not found.
 *
 * The `key` parameter must be null-terminated. Returned pointer will be a
 * sentinel if the map's `value_size` is zero, and it should not be
 * dereferenced.
 */
void *lf(map_get)(struct lf(map) *map, const void *key);

/** @brief Identical to map_get(), but accepts a non-null-terminated key. */
void *lf(map_get2)(struct lf(map) *map, const void *key, size_t keylen);

/**
 * @brief Inserts a key-value pair into the map.
 *
 * @warning The key must not already exist in the map.
 */
int lf(map_insert)(struct lf(map) *map,
		   const void *key,
		   const void *value) lfi_wur;

/** @brief Identical to map_insert(), but raises an error if memory allocation
 * fails. */
void lf(map_xinsert)(struct lf(map) *map, const void *key, const void *value);

/** @brief Identical to map_insert(), but accepts a non-null-terminated key. */
int lf(map_insert2)(struct lf(map) *map,
		    const void *key,
		    size_t keylen,
		    const void *value);

/** @brief Identical to map_insert2(), but raises an error if memory allocation
 * fails. */
void lf(map_xinsert2)(struct lf(map) *map,
		      const void *key,
		      size_t keylen,
		      const void *value);

/**
 * @brief Removes a key-value pair from the map and returns a pointer to the value.
 *
 * @attention The returned value pointer points to internal memory that is only
 * valid until the next remove operation. The user must copy the underlying
 * data if they wish to retain it.
 */
void *lf(map_remove)(struct lf(map) *map, const void *key);

/** @brief Identical to map_remove(), but accepts a non-null-terminated key. */
void *lf(map_remove2)(struct lf(map) *map, const void *key, size_t keylen);

/**
 * @brief Retrieves the map entry at a specific sorted index.
 *
 * Raises an error if the index is larger than map size. The entry may be
 * invalidated after a remove operation.
 */
struct lf(map_entry) lf(map_select)(struct lf(map) *map, ptrdiff_t index);

/**
 * @brief Determines the 0-based index of a specific key in the sorted map.
 *
 * Returns -1 casted to size_t if the key is not found.
 */
size_t lf(map_rank)(const struct lf(map) *map, const void *key);

/** @brief Identical to map_rank(), but accepts a non-null-terminated key. */
size_t lf(map_rank2)(const struct lf(map) *map, const void *key, size_t keylen);

/** @brief Returns the total number of elements currently stored in the map. */
size_t lf(map_size)(const struct lf(map) *map);

/**
 * @brief Creates a forward iteration handle for the map.
 *
 * Initializes `it` to iterate over all entries in ascending key order. The
 * first call to map_iter_next() will return the entry with the smallest key.
 *
 * @attention The iterator is invalidated by any insert or remove operation
 * on the map. Do not modify the map while iterating.
 */
void lf(map_iter)(struct lf(map) *map, struct lf(map_it) *it);

/**
 * @brief Creates a reverse handle from specific index for the map.
 *
 * See map_iter().
 */
void lf(map_iter_from)(struct lf(map) *map,
		       struct lf(map_it) *it,
		       ptrdiff_t index);

/**
 * @brief Retrieves the next entry from a forward iteration handle.
 *
 * Returns entries in ascending key order. When all entries have been
 * retrieved, returns a sentinel entry with `.key == NULL` and `.keylen == 0`.
 */
struct lf(map_entry) lf(map_iter_next)(struct lf(map_it) *it);

/**
 * @brief Retrieves the next entry from a reverse iteration handle.
 *
 * Returns entries in descending key order. When all entries have been
 * retrieved, returns a sentinel entry with `.key == NULL` and `.keylen == 0`.
 */
struct lf(map_entry) lf(map_iter_prev)(struct lf(map_it) *it);


#endif
