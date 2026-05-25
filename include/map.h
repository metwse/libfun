/**
 * @file map.h
 * @brief Basic hashmap.
 *
 * map uses the FNV hash function and open addressing method. It stores
 * data as key-value pairs, allowing variable-length keys while storing
 * fixed-length values.
 */

#ifndef LF_MAP_H
#define LF_MAP_H

#ifndef LF_HEADERONLY
#include "config.h"
#endif

#include <assert.h>
#include <stddef.h>


/** @brief map. */
struct lf(map) {
	/** @cond */
	struct lf(map_entry) *entries;
	size_t cap;
	size_t used;
	size_t value_size;
	/** @endcond */
};

/** @brief Iteration handle to retrieve map entries one by one. */
struct lf(map_it) {
	/** @cond */
	struct lf(map) *m;
	size_t i;
	/** @endcond */
};

/** @brief Key-value pair stored in the map, entry. */
struct lf(map_entry) {
	/** Key, the unique identifier pointing to the value. */
	const void *key;
	/** Length of the data stored in the key. */
	size_t keylen;
	/** The value assigned to the key by the user. */
	char value[];
};


/**
 * @brief Creates a new map.
 *
 * Allocates the necessary memory for the map. The `value_size` parameter
 * specifies the maximum size of the `value`s the user will add.
 *
 * Returns non-zero if the a memory allocation failure occurs.
 */
int lf(map_init)(struct lf(map) *map, size_t value_size) lfi_wur;

/** @brief Identical to map_init(), but raises an error if memory allocation
 * fails. */
void lf(map_xinit)(struct lf(map) *map, size_t value_size);

/** @brief Clears the memory allocated by the map. */
void lf(map_destroy)(struct lf(map) *map);

/**
 * @brief Returns a pointer to the value matching the key, returns `NULL` if
 * the key is not found.
 *
 * The `key` parameter must be null-terminated.
 */
void *lf(map_get)(struct lf(map) *map, const void *key);

/**
 * @brief Returns a pointer to the value matching the key, returns `NULL` if
 * the key is not found.
 */
void *lf(map_get2)(struct lf(map) *map, const void *key, size_t keylen);

/**
 * @brief Inserts a key-value pair into the map.
 *
 * @warning The `key` must not already exist in the map.
 *
 * The `key` parameter must be null-terminated. Returns non-zero if the a
 * memory allocation failure occurs.
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
		    const void *value) lfi_wur;

/** @brief Identical to map_insert2(), but raises an error if memory allocation
 * fails. */
void lf(map_xinsert2)(struct lf(map) *map,
		      const void *key,
		      size_t keylen,
		      const void *value);

/**
 * @brief Removes the key-value pair from the map, returns a pointer to the
 * value.
 *
 * @attention The returned value pointer is valid until the next map operation.
 * If the user wants to continue using the value, they must copy underlying
 * data.
 *
 * The `key` parameter must be `null-terminated`.
 */
void *lf(map_remove)(struct lf(map) *map, const void *key);

/** @brief Identical to map_remove(), but accepts a non-null-terminated key. */
void *lf(map_remove2)(struct lf(map) *map, const void *key, size_t keylen);

/** @brief Creates an iteration handle to retrieve the entries in the map one
 * by one. */
void lf(map_iter)(struct lf(map) *map, struct lf(map_it) *it);

/**
 * @brief Retrieves the next entry from the iteration handle.
 *
 * If all entries have been retrieved, it returns `NULL`.
 */
struct lf(map_entry) *lf(map_iter_next)(struct lf(map_it) *it);


#endif
