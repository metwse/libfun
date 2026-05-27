#ifndef LF_HEADERONLY
#include "util.h"
#include "../include/map.h"
#endif

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

lfi_fdecl(struct lfi(map_node) *, map_select_node)(struct lf(map) *m,
						   ptrdiff_t i_)
{
	size_t i = lfi(circular_index)(i_, lf(map_size)(m));

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

	return cur;
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
lfi_fdecl(struct lf(entry), map_entry_of)(struct lfi(map_node) *n)
{
	if (n == NULL)
		return lfi_sentinel_entry;

	return (struct lf(entry)) {
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
	lf_unwrap(lf(map_init)(m, value_size, cmp));
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

void *lf(map_insert)(struct lf(map) *m, const void *key, const void *value)
{
	return lf(map_insert2)(m, key, strlen(key), value);
}

void lf(map_xinsert)(struct lf(map) *m, const void *key, const void *value)
{
	lf_assert(lf(map_insert)(m, key, value),);
}

void *lf(map_insert2)(struct lf(map) *m,
		      const void *key,
		      size_t keylen,
		      const void *value)
{
	struct lfi(map_node) *n =
		lfi(map_new_node)(key, keylen, value, m->value_size);

	if (n == NULL)
		return NULL;

	if (m->root == NULL) {
		m->root = n;
	} else {
		struct lfi(map_node) *cur = m->root;

		struct lfi(map_node) *p;
		int cmp = 0;

		while (cur != NULL) {
			p = cur;
			cur->size++;

			cmp = m->cmp(cur->kv, key, cur->keylen, keylen);

			lf_assert(cmp != 0, "map already contains the element");

			if (cmp < 0)
				cur = cur->right;
			else if (cmp > 0)
				cur = cur->left;
		}

		if (cmp < 0)
			p->right = n;
		else if (cmp > 0)
			p->left = n;

		n->p = p;
	}

	lfi(map_insert_fixup)(m, n);

	return lf_map_node_value(n);
}

void lf(map_xinsert2)(struct lf(map) *m,
		      const void *key,
		      size_t keylen,
		      const void *value)
{
	lf_assert(lf(map_insert2)(m, key, keylen, value),);
}

const void *lf(map_remove)(struct lf(map) *m, const void *key)
{
	return lf(map_remove2)(m, key, strlen(key));
}

const void *lf(map_remove2)(struct lf(map) *m, const void *key, size_t keylen)
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

struct lf(entry) lf(map_select)(struct lf(map) *m, ptrdiff_t i)
{
	return lfi(map_entry_of)(lfi(map_select_node)(m, i));
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
	lf(map_iter_from)(m, it, 0);
}

void lf(map_iter_from)(struct lf(map) *m,
		       struct lf(map_it) *it,
		       ptrdiff_t i)
{
	it->m = m;
	it->n = lfi(map_select_node)(m, i);
}

struct lf(entry) lf(map_iter_next)(struct lf(map_it) *it)
{
	struct lfi(map_node) *cur = it->n;

	if (cur != NULL)
		it->n = lfi(map_successor)(cur);

	return lfi(map_entry_of)(cur);
}

struct lf(entry) lf(map_iter_prev)(struct lf(map_it) *it)
{
	struct lfi(map_node) *cur = it->n;

	if (cur != NULL)
		it->n = lfi(map_predecessor)(cur);

	return lfi(map_entry_of)(cur);
}
