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

	if (value_size > 0)
		memcpy(&n->kv[aligned_keylen], value, value_size);

	n->keylen = keylen;
	n->p = n->left = n->right = NULL;
	n->size = 1;
	n->color = 1;

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

lfi_fdecl(void, map_fixup)(struct lf(map) *m, struct lfi(map_node) *z)
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


int lf(map_init)(struct lf(map) *m,
		 size_t value_size,
		 int (*cmp)(const void *, const void *, size_t, size_t))
{
	m->root = NULL;
	m->value_size = value_size;
	m->cmp = cmp == NULL ? lfi(map_default_comparator) : cmp;
	m->hold_value = malloc(value_size);

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

	free(m->hold_value);
}

void *lf(map_get)(struct lf(map) *m, const void *key)
{
	return lf(map_get2)(m, key, strlen(key));
}

void *lf(map_get2)(struct lf(map) *m, const void *key, size_t keylen)
{
	struct lfi(map_node) *cur = m->root;

	while (cur != NULL) {
		int cmp = m->cmp(cur->kv, key, cur->keylen, keylen);

		if (cmp < 0)
			cur = cur->right;
		else if (cmp > 0)
			cur = cur->left;
		else
			return &cur->kv[lf_map_align(keylen)];
	}

	return NULL;
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

		goto return_fixup;
	}

	struct lfi(map_node) *p;
	struct lfi(map_node) *cur = m->root;

	int cmp;

	while (cur != NULL) {
		p = cur;
		cur->size++;

		cmp = m->cmp(cur->kv, key, cur->keylen, keylen);

		assert(cmp != 0 && "map already contains the element");

		if (cmp < 0)
			cur = cur->right;
		else if (cmp > 0)
			cur = cur->left;
	}

	if (cmp < 0)
		p->right = n;
	else
		p->left = n;

	n->p = p;

return_fixup:
	lfi(map_fixup)(m, n);

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
	(void) m; (void) key; (void) keylen;
	assert(0);
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

	return (struct lf(map_entry)) {
		.key = cur->kv,
		.keylen = cur->keylen,
		.value = &cur->kv[lf_map_align(cur->keylen)],
	};
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
