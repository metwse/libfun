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

#include <assert.h>
#include <stddef.h>


/** @brief map. */
struct map {
	/** @cond */
	struct map_node *root;

	void *hold_value;

	size_t value_size;

	int (*cmp)(const void *, const void *, size_t, size_t);
	/** @endcond */
};

/** @brief Map entry. */
struct map_entry {
	/** Key, the unique identifier pointing to the value. */
	const void *key;
	/** Length of the data stored in the key. */
	size_t keylen;
	/** The value assigned to the key by the user. */
	const void *value;
};

int map_init(struct map *map, size_t value_size,
	     int (*comparator)(const void *key1,
			       const void *key2,
			       size_t keylen1,
			       size_t keylen2));


void map_destroy(struct map *map);

void *map_get2(struct map *map, const void *key, size_t keylen);

struct map_entry map_select(struct map *map, size_t index);

size_t map_rank(const struct map *map, const void *key, size_t keylen);

size_t map_size(const struct map *map);

int map_insert2(struct map *map,
		const void *key,
		size_t keylen,
		const void *value);

void *map_remove2(struct map *map, const void *key, size_t keylen);


#endif
