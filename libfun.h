#define LF_HEADERONLY
/**
 * @file config.h
 * @brief Library-wide configuration.
 */

#ifndef LF_HASHMAP_INITIAL_CAP
/** @brief Initial capacity of the hashmap. */
#define LF_HASHMAP_INITIAL_CAP 64
#endif

#ifndef LF_STACK_INITIAL_CAP
/** @brief Initial capacity of the stack. */
#define LF_STACK_INITIAL_CAP 64
#endif

/** Public function prefixing */
#ifndef LIBFUN_PREFIX
/** @brief Function prefix. */
#define LIBFUN_PREFIX f
#endif

#ifndef lf
/** @cond */
#define lfi_concat_inner(prefix, name) prefix ## name
#define lfi_concat(prefix, name) lfi_concat_inner(prefix, name)
/** @endcond */

/* libfun public symbols. */
#define lf(name) lfi_concat(LIBFUN_PREFIX, name)
#endif


/** @cond */
#ifndef lfi_wur
#if defined(__GNUC__) || defined(__clang__)
#define lfi_wur __attribute__((warn_unused_result))
#else
#define lfi_wur
#endif
#endif

#ifndef lfi
/* Internal function declaration & name prefixing. */
#define lfi(name) lf(_libfun_internal_ ## name)

#define lfi_fdecl(ret_ty, name) static ret_ty lfi(name)
#endif
/** @endcond */
/**
 * @file stack.h
 * @brief Basic stack.
 *
 * Generally, stacks do not allow random access; an element in the stack is not
 * accessed by its index. Strict stack implementations only allow access to the
 * top element (`pop`/`peek`); they even hide the total number of elements and
 * only provide a function indicating whether the stack is empty.
 *
 * This stack implementation is more flexible:
 * - You can get a reference to any element.
 * - You can get the total number of elements.
 *
 * However, you can still only push to and pop from the top of the stack.
 */

#ifndef LF_STACK_H
#define LF_STACK_H

#ifndef LF_HEADERONLY
#include "config.h"
#endif

#include <stddef.h>


/** @brief stack. */
struct lf(stack) {
    /** @cond */
    char *data;
    size_t cap;
    size_t len;
    size_t item_size;
    /** @endcond */
};


/**
 * @brief Creates a new stack.
 *
 * Allocates the necessary memory for the stack. The `item_size` parameter
 * specifies the size of the elements the user will add.
 *
 * Returns non-zero if a memory allocation failure occurs.
 */
int lf(stack_init)(struct lf(stack) *stack, size_t item_size) lfi_wur;

/** @brief Identical to stack_init(), but raises an error if memory allocation
 * fails. */
void lf(stack_xinit)(struct lf(stack) *map, size_t value_size);

/** @brief Clears the memory allocated by the stack. */
void lf(stack_destroy)(struct lf(stack) *stack);

/** @brief Removes and returns the top element from the stack. */
void *lf(stack_pop)(struct lf(stack) *stack);

/** @brief Pushes an element to the top of the stack. */
int lf(stack_push)(struct lf(stack) *stack, void *item) lfi_wur;

/** @brief Identical to stack_push(), but raises an error if memory allocation
 * fails. */
void lf(stack_xpush)(struct lf(stack) *stack, void *item);

/** @brief Returns the top element of the stack. */
void *lf(stack_top)(struct lf(stack) *stack);

/** @brief Returns the element at the specified `index`. */
void *lf(stack_at)(struct lf(stack) *stack, size_t index);

/** @brief Returns the total number of elements. */
size_t lf(stack_len)(const struct lf(stack) *stack);


#endif
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
 * The `key` parameter must be null-terminated. Returned pointer will be a
 * sentinel if the map's `value_size` is zero, and it should not be
 * dereferenced.
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
 * @brief Creates a reverse iteration handle for the map.
 *
 * Initializes `it` to iterate over all entries in descending key order. The
 * first call to map_iter_prev() will return the entry with the largest key.
 *
 * @attention The iterator is invalidated by any insert or remove operation
 * on the map. Do not modify the map while iterating.
 */
void lf(map_iter_rev)(struct lf(map) *map, struct lf(map_it) *it);

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
#ifdef LF_IMPLEMENTATION
#ifndef LF_HEADERONLY
#include "../include/config.h"
#include "../include/hashmap.h"
#endif

#include <assert.h>
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
	assert(lf(hashmap_init)(m, value_size) == 0);
}

void lf(hashmap_xinsert)(struct lf(hashmap) *m,
			 const void *key,
			 const void *value)
{
	assert(lf(hashmap_insert)(m, key, value) == 0);
}

void lf(hashmap_xinsert2)(struct lf(hashmap) *m,
			  const void *key,
			  size_t keylen,
			  const void *value)
{
	assert(lf(hashmap_insert2)(m, key, keylen, value) == 0);
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

	assert(!lf(hashmap_get2)(m, key, keylen) && "hashmap contains the element");

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

	assert(0);  // GCOVR_EXCL_LINE: unreachable
}
#ifndef LF_HEADERONLY
#include "../include/map.h"
#endif

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define lf_map_align(i) ((i + sizeof(size_t) - 1) / sizeof(size_t) * sizeof(size_t))

#define lf_map_node_size(n) ((n) == NULL ? 0 : (n)->size)

#define lf_map_node_color(n) ((n) == NULL ? 0 : (n)->color)

#define lf_map_node_value(n) (&(n)->kv[lf_map_align((n)->keylen)])


/* Allocates a new node. */
lfi_fdecl(struct lfi(map_node) *, map_new_node)(const void *key,
						size_t keylen,
						const void *value,
						size_t value_size)
{
	size_t size = lf_map_align(sizeof(struct lfi(map_node)));
	size_t aligned_keylen = lf_map_align(keylen);

	if (value_size > 0)
		size += aligned_keylen + value_size;
	else
		size += keylen;

	struct lfi(map_node) *n = malloc(size);

	if (n == NULL)
		return NULL;

	memcpy(n->kv, key, keylen);

	n->keylen = keylen;
	n->p = n->left = n->right = NULL;
	n->size = 1;
	n->color = 1;

	if (value_size > 0)
		memcpy(lf_map_node_value(n), value, value_size);

	return n;
}

lfi_fdecl(void, map_left_rotate)(struct lf(map) *m, struct lfi(map_node) *x)
{
	struct lfi(map_node) *y = x->right;
	x->right = y->left;

	if (y->left != NULL)
		y->left->p = x;

	y->p = x->p;
	if (x->p == NULL)
		m->root = y;
	else if (x == x->p->left)
		x->p->left = y;
	else
		x->p->right = y;

	y->left = x;
	x->p = y;

	y->size = x->size;
	x->size = lf_map_node_size(x->left) + lf_map_node_size(x->right) + 1;
}

lfi_fdecl(void, map_right_rotate)(struct lf(map) *m, struct lfi(map_node) *x)
{
	struct lfi(map_node) *y = x->left;
	x->left = y->right;

	if (y->right != NULL)
		y->right->p = x;

	y->p = x->p;
	if (x->p == NULL)
		m->root = y;
	else if (x == x->p->right)
		x->p->right = y;
	else
		x->p->left = y;

	y->right = x;
	x->p = y;

	y->size = x->size;
	x->size = lf_map_node_size(x->left) + lf_map_node_size(x->right) + 1;
}

lfi_fdecl(void, map_delete_fixup)(struct lf(map) *m,
				  struct lfi(map_node) *x,
				  struct lfi(map_node) *x_parent)
{
	while (x != m->root && lf_map_node_color(x) == 0) {
		if (x == x_parent->left) {
			struct lfi(map_node) *w = x_parent->right;

			if (lf_map_node_color(w) == 1) {
				/* case 1 */
				w->color = 0;
				x_parent->color = 1;
				lfi(map_left_rotate)(m, x_parent);
				w = x_parent->right;
			}

			if (lf_map_node_color(w->left) == 0 &&
				lf_map_node_color(w->right) == 0) {
				/* case 2 */
				w->color = 1;
				x = x_parent;
				x_parent = x->p;
			} else {
				if (lf_map_node_color(w->right) == 0) {
					/* case 3 */
					if (w->left) w->left->color = 0;
					w->color = 1;
					lfi(map_right_rotate)(m, w);
					w = x_parent->right;
				}

				/* case 4 */
				w->color = x_parent->color;
				x_parent->color = 0;
				if (w->right)
					w->right->color = 0;
				lfi(map_left_rotate)(m, x_parent);
				x = m->root;
			}
		} else {
			struct lfi(map_node) *w = x_parent->left;

			if (lf_map_node_color(w) == 1) {
				/* case 1 */
				w->color = 0;
				x_parent->color = 1;
				lfi(map_right_rotate)(m, x_parent);
				w = x_parent->left;
			}

			if (lf_map_node_color(w->right) == 0 &&
				lf_map_node_color(w->left) == 0) {
				/* case 2 */
				w->color = 1;
				x = x_parent;
				x_parent = x->p;
			} else {
				if (lf_map_node_color(w->left) == 0) {
					/* case 3 */
					if (w->right)
						w->right->color = 0;
					w->color = 1;
					lfi(map_left_rotate)(m, w);
					w = x_parent->left;
				}

				/* case 4 */
				w->color = x_parent->color;
				x_parent->color = 0;
				if (w->left)
					w->left->color = 0;
				lfi(map_right_rotate)(m, x_parent);
				x = m->root;
			}
		}
	}

	if (x != NULL)
		x->color = 0;
}

lfi_fdecl(void, map_insert_fixup)(struct lf(map) *m, struct lfi(map_node) *z)
{
	while (z->p != NULL && z->p->color == 1) {
		if (z->p == z->p->p->left) {
			struct lfi(map_node) *y = z->p->p->right;  // Uncle

			if (y != NULL && y->color == 1) {
				/* case 1 */
				z->p->color = 0;
				y->color = 0;
				z->p->p->color = 1;
				z = z->p->p;
			} else {
				if (z == z->p->right) {
					/* case 2 */
					z = z->p;
					lfi(map_left_rotate)(m, z);
				}

				/* case 3 */
				z->p->color = 0;
				z->p->p->color = 1;
				lfi(map_right_rotate)(m, z->p->p);
			}
		} else {
			struct lfi(map_node) *y = z->p->p->left;  // Uncle

			if (y != NULL && y->color == 1) {
				z->p->color = 0;
				y->color = 0;
				z->p->p->color = 1;
				z = z->p->p;
			} else {
				if (z == z->p->left) {
					z = z->p;
					lfi(map_right_rotate)(m, z);
				}

				z->p->color = 0;
				z->p->p->color = 1;
				lfi(map_left_rotate)(m, z->p->p);
			}
		}
	}

	/* case 0 */
	m->root->color = 0;
}

lfi_fdecl(void, map_transplant)(struct lf(map) *m,
			    struct lfi(map_node) *u,
			    struct lfi(map_node) *v)
{
	if (u->p == NULL)
		m->root = v;
	else if (u == u->p->left)
		u->p->left = v;
	else
		u->p->right = v;

	if (v != NULL)
		v->p = u->p;
}

/* Default comparator used if no comparatasion function is given to the
 * map_init(). */
lfi_fdecl(int, map_default_comparator)(const void *key1, const void *key2,
				       size_t keylen1, size_t keylen2)
{
	int res = memcmp(key1, key2, keylen1 < keylen2 ? keylen1 : keylen2);

	if (res == 0) {
		if (keylen1 == keylen2)
			return 0;
		else if (keylen1 < keylen2)
			return -1;
		else
			return 1;
	}

	return res;
}

lfi_fdecl(void, map_destroy_recursive)(struct lf(map) *m,
				       struct lfi(map_node) *n)
{
	if (n != NULL) {
		lfi(map_destroy_recursive)(m, n->left);
		lfi(map_destroy_recursive)(m, n->right);

		free(n);
	}
}

lfi_fdecl(struct lfi(map_node) *, map_get2_node)(struct lf(map) *m,
						 const void *key,
						 size_t keylen)
{
	struct lfi(map_node) *cur = m->root;

	while (cur != NULL) {
		int cmp = m->cmp(cur->kv, key, cur->keylen, keylen);

		if (cmp < 0)
			cur = cur->right;
		else if (cmp > 0)
			cur = cur->left;
		else
			return cur;
	}

	return NULL;
}

/* Returns the leftmost node in the subtree rooted at n. */
lfi_fdecl(struct lfi(map_node) *, map_leftmost)(struct lfi(map_node) *n)
{
	if (n == NULL)
		return NULL;

	while (n->left)
		n = n->left;

	return n;
}

/* Returns the rightmost node in the subtree rooted at n. */
lfi_fdecl(struct lfi(map_node) *, map_rightmost)(struct lfi(map_node) *n)
{
	if (n == NULL)
		return NULL;

	while (n->right)
		n = n->right;

	return n;
}

/* Returns the in-order successor of n, or NULL if n is the maximum. */
lfi_fdecl(struct lfi(map_node) *, map_successor)(struct lfi(map_node) *n)
{
	if (n->right)
		return lfi(map_leftmost)(n->right);

	struct lfi(map_node) *y = n->p;

	while (y != NULL && n == y->right) {
		n = y;
		y = y->p;
	}

	return y;
}

/* Returns the in-order predecessor of n, or NULL if n is the minimum. */
lfi_fdecl(struct lfi(map_node) *, map_predecessor)(struct lfi(map_node) *n)
{
	if (n->left)
		return lfi(map_rightmost)(n->left);

	struct lfi(map_node) *y = n->p;

	while (y != NULL && n == y->left) {
		n = y;
		y = y->p;
	}

	return y;
}

/* Constructs a map_entry from a node, or the exhaustion sentinel if NULL. */
lfi_fdecl(struct lf(map_entry), map_entry_of)(struct lfi(map_node) *n)
{
	if (n == NULL)
		return (struct lf(map_entry)) {
			.key = NULL,
			.keylen = 0,
			.value = NULL
		};

	return (struct lf(map_entry)) {
		.key = n->kv,
		.keylen = n->keylen,
		.value = lf_map_node_value(n),
	};
}


int lf(map_init)(struct lf(map) *m,
		 size_t value_size,
		 int (*cmp)(const void *, const void *, size_t, size_t))
{
	m->root = NULL;
	m->value_size = value_size;
	m->cmp = cmp == NULL ? lfi(map_default_comparator) : cmp;

	if (m->value_size)
		m->hold_value = malloc(value_size);
	else
		m->hold_value = (void *) 1;

	return m->hold_value == NULL ? 1 : 0;
}

void lf(map_xinit)(struct lf(map) *m,
		   size_t value_size,
		   int (*cmp)(const void *, const void *, size_t, size_t))
{
	assert(lf(map_init)(m, value_size, cmp) == 0);
}

void lf(map_destroy)(struct lf(map) *m)
{
	lfi(map_destroy_recursive)(m, m->root);

	if (m->value_size)
		free(m->hold_value);
}

void *lf(map_get)(struct lf(map) *m, const void *key)
{
	return lf(map_get2)(m, key, strlen(key));
}

void *lf(map_get2)(struct lf(map) *m, const void *key, size_t keylen)
{
	struct lfi(map_node) *n = lfi(map_get2_node)(m, key, keylen);

	return n != NULL ? lf_map_node_value(n) : NULL;
}

int lf(map_insert)(struct lf(map) *m, const void *key, const void *value)
{
	return lf(map_insert2)(m, key, strlen(key), value);
}

void lf(map_xinsert)(struct lf(map) *m, const void *key, const void *value)
{
	assert(lf(map_insert)(m, key, value) == 0);
}

int lf(map_insert2)(struct lf(map) *m,
		    const void *key,
		    size_t keylen,
		    const void *value)
{
	struct lfi(map_node) *n =
		lfi(map_new_node)(key, keylen, value, m->value_size);

	if (n == NULL)
		return 1;

	if (m->root == NULL) {
		m->root = n;
	} else {
		struct lfi(map_node) *p;
		struct lfi(map_node) *cur = m->root;

		int cmp = 0;

		while (cur != NULL) {
			p = cur;
			cur->size++;

			cmp = m->cmp(cur->kv, key, cur->keylen, keylen);

			assert(cmp != 0 && "map already contains the element");

			if (cmp < 0)
				cur = cur->right;
			else if (cmp > 0)
				cur = cur->left;
			else
				abort();
		}

		if (cmp < 0)
			p->right = n;
		else if (cmp > 0)
			p->left = n;

		n->p = p;
	}

	lfi(map_insert_fixup)(m, n);

	return 0;
}

void lf(map_xinsert2)(struct lf(map) *m,
		      const void *key,
		      size_t keylen,
		      const void *value)
{
	assert(lf(map_insert2)(m, key, keylen, value) == 0);
}

void *lf(map_remove)(struct lf(map) *m, const void *key)
{
	return lf(map_remove2)(m, key, strlen(key));
}

void *lf(map_remove2)(struct lf(map) *m, const void *key, size_t keylen)
{
	struct lfi(map_node) *z = lfi(map_get2_node)(m, key, keylen);

	if (z == NULL)
		return NULL;

	if (m->value_size)
		memcpy(m->hold_value, lf_map_node_value(z), m->value_size);

	struct lfi(map_node) *y = z;

	char orig_color = y->color;

	struct lfi(map_node) *x, *x_parent = NULL;

	if (z->left == NULL) {
		/* case 1 */
		x = z->right;
		x_parent = z->p;
		lfi(map_transplant)(m, z, z->right);
	} else if (z->right == NULL) {
		/* case 2 */
		x = z->left;
		x_parent = z->p;
		lfi(map_transplant)(m, z, z->left);
	} else {
		/* case 3 */
		y = z->right;

		while (y->left)
			y = y->left;

		orig_color = y->color;
		x = y->right;

		if (y->p == z) {
			x_parent = y;
			if (x != NULL)
				x->p = y;
		} else {
			x_parent = y->p;
			lfi(map_transplant)(m, y, y->right);
			y->right = z->right;
			y->right->p = y;
		}

		lfi(map_transplant)(m, z, y);
		y->left = z->left;
		y->left->p = y;
		y->color = z->color;

		y->size = z->size;
	}

	struct lfi(map_node) *cur = x_parent;
	while (cur != NULL) {
		cur->size--;
		cur = cur->p;
	}

	if (orig_color == 0)
		lfi(map_delete_fixup)(m, x, x_parent);

	free(z);

	return m->hold_value;
}

struct lf(map_entry) lf(map_select)(struct lf(map) *m, size_t i)
{
	assert(i < lf(map_size)(m) && "map overflow");

	struct lfi(map_node) *cur = m->root;

	while (true) {
		if (lf_map_node_size(cur->left) == i)
			break;

		if (lf_map_node_size(cur->left) > i) {
			cur = cur->left;
		} else {
			i -= lf_map_node_size(cur->left) + 1;
			cur = cur->right;
		}
	}

	return lfi(map_entry_of)(cur);
}

size_t lf(map_size)(const struct lf(map) *m)
{
	return lf_map_node_size(m->root);
}

size_t lf(map_rank)(const struct lf(map) *m, const void *key)
{
	return lf(map_rank2)(m, key, strlen(key));
}

size_t lf(map_rank2)(const struct lf(map) *m, const void *key, size_t keylen)
{
	struct lfi(map_node) *cur = m->root;

	size_t rank = 0;

	while (cur != NULL) {
		int cmp = m->cmp(cur->kv, key, cur->keylen, keylen);

		if (cmp < 0) {
			rank += lf_map_node_size(cur->left) + 1;

			cur = cur->right;
		} else if (cmp > 0) {
			cur = cur->left;
		} else {
			rank += lf_map_node_size(cur->left);

			return rank;
		}
	}

	return -1;
}

void lf(map_iter)(struct lf(map) *m, struct lf(map_it) *it)
{
	it->m = m;
	it->n = lfi(map_leftmost)(m->root);
}

void lf(map_iter_rev)(struct lf(map) *m, struct lf(map_it) *it)
{
	it->m = m;
	it->n = lfi(map_rightmost)(m->root);
}

struct lf(map_entry) lf(map_iter_next)(struct lf(map_it) *it)
{
	struct lfi(map_node) *cur = it->n;

	if (cur != NULL)
		it->n = lfi(map_successor)(cur);

	return lfi(map_entry_of)(cur);
}

struct lf(map_entry) lf(map_iter_prev)(struct lf(map_it) *it)
{
	struct lfi(map_node) *cur = it->n;

	if (cur != NULL)
		it->n = lfi(map_predecessor)(cur);

	return lfi(map_entry_of)(cur);
}
#ifndef LF_HEADERONLY
#include "../include/config.h"
#include "../include/stack.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>


int lf(stack_init)(struct lf(stack) *s, size_t item_size)
{
	s->cap = LF_STACK_INITIAL_CAP;
	s->len = 0;
	s->item_size = item_size;
	s->data = malloc(item_size * LF_STACK_INITIAL_CAP);

	return s->data == NULL ? 1 : 0;
}

void lf(stack_xinit)(struct lf(stack) *s, size_t item_size)
{
	assert(lf(stack_init)(s, item_size) == 0);
}

void lf(stack_destroy)(struct lf(stack) *s)
{
	free(s->data);
}

void *lf(stack_pop)(struct lf(stack) *s)
{
	assert(s->len && "stack underflow");

	void *item = lf(stack_at)(s, s->len - 1);

	s->len--;

	return item;
}

int lf(stack_push)(struct lf(stack) *s, void *item)
{
	if (s->len == s->cap) {
		s->cap *= 2;

		s->data = realloc(s->data,
					s->cap * s->item_size);

		if (s->data == NULL)
			return 1;
	}

	memcpy(&s->data[s->len * s->item_size],
	       item, s->item_size);

	s->len++;

	return 0;
}

void lf(stack_xpush)(struct lf(stack) *s, void *item)
{
	assert(lf(stack_push)(s, item) == 0);
}

void *lf(stack_top)(struct lf(stack) *s)
{
	assert(s->len && "stack underflow");

	return lf(stack_at)(s, s->len - 1);
}

void *lf(stack_at)(struct lf(stack) *s, size_t index)
{
	return &s->data[index * s->item_size];
}

size_t lf(stack_len)(const struct lf(stack) *s)
{
	return s->len;
}
#endif
