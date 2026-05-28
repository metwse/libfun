/**
 * @file common.h
 * @brief Library-wide common types.
 */

#ifndef LF_COMMON_H
#define LF_COMMON_H

#ifndef LF_HEADERONLY
#include "config.h"
#endif

#include <stdbool.h>
#include <stddef.h>


/** @brief Key-value pair stored in the hashmap, entry. */
struct lf(entry) {
	/** Key, the unique identifier pointing to the value. */
	const void *key;

	/** Length of the key. */
	size_t keylen;

	/** The value assigned to the key by the user. */
	void *value;
};


/** @brief Returns false if the entry is sentinel or invalid. */
static inline bool lf(entry_is_valid)(struct lf(entry) entry)
{
	return entry.key != NULL;
}


#endif
