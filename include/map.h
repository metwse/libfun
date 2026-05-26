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

/** @brief Map entry. */
struct lf(map_entry) {
	/** Key, the unique identifier pointing to the value. */
	const void *key;
	/** Length of the data stored in the key. */
	size_t keylen;
	/** The value assigned to the key by the user. */
	const void *value;
};


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
 * The `key` parameter must be null-terminated.
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
void *lf(map_remove)(struct lf(map) *map, const void *key) lfi_wur;

/** @brief Identical to map_remove(), but accepts a non-null-terminated key. */
void *lf(map_remove2)(struct lf(map) *map, const void *key, size_t keylen) lfi_wur;

/**
 * @brief Retrieves the map entry at a specific sorted index.
 *
 * Raises an error if the index is larger than map size.
 */
struct lf(map_entry) lf(map_select)(struct lf(map) *map, size_t index);

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


#endif
