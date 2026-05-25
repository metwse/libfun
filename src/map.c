#include <assert.h>
#include <string.h>
#ifndef LF_HEADERONLY
#include "../include/map.h"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>


#define lf_map_align(i) ((i + sizeof(size_t) - 1) / sizeof(size_t) * sizeof(size_t))

#define lf_map_node_size(n) ((n) == NULL ? 0 : (n)->size)


struct map_node {
	size_t keylen;

	struct map_node *parent;
	struct map_node *left;
	struct map_node *right;

	/* Number of nodes in the subtree rooted at this node *including*
	 * this node. */
	size_t size;

	/* 1 if red, 0 if black. */
	char color;

	/* Key and value. */
	char kv[];
};

static struct map_node *map_new_node(const void *key, size_t keylen,
				     const void *value, size_t value_size)
{
	size_t size = lf_map_align(sizeof(struct map_node));
	size_t aligned_keylen = lf_map_align(keylen);

	if (value_size > 0)
		size += aligned_keylen + value_size;
	else
		size += keylen;

	struct map_node *n = malloc(size);

	if (n == NULL)
		return NULL;

	memcpy(n->kv, key, keylen);

	if (value_size > 0)
		memcpy(&n->kv[aligned_keylen], value, value_size);

	n->keylen = keylen;
	n->parent = n->left = n->right = NULL;
	n->size = 1;
	n->color = 1;

	return n;
}

static void map_left_rotate(struct map *m, struct map_node *x)
{
	struct map_node *y = x->right;
	x->right = y->left;

	if (y->left != NULL)
		y->left->parent = x;

	y->parent = x->parent;
	if (x->parent == NULL)
		m->root = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;

	y->left = x;
	x->parent = y;

	y->size = x->size;
	x->size = lf_map_node_size(x->left) + lf_map_node_size(x->right) + 1;
}

static void map_right_rotate(struct map *m, struct map_node *x)
{
	struct map_node *y = x->left;
	x->left = y->right;

	if (y->right != NULL)
		y->right->parent = x;

	y->parent = x->parent;
	if (x->parent == NULL)
		m->root = y;
	else if (x == x->parent->right)
		x->parent->right = y;
	else
		x->parent->left = y;

	y->right = x;
	x->parent = y;

	y->size = x->size;
	x->size = lf_map_node_size(x->left) + lf_map_node_size(x->right) + 1;
}

static void map_fixup(struct map *m, struct map_node *z)
{
	while (z->parent != NULL && z->parent->color == 1) {
		if (z->parent == z->parent->parent->left) {
			struct map_node *y = z->parent->parent->right;  // Uncle

			if (y != NULL && y->color == 1) {
				/* case 1 */
				z->parent->color = 0;
				y->color = 0;
				z->parent->parent->color = 1;
				z = z->parent->parent;
			} else {
				if (z == z->parent->right) {
					/* case 2 */
					z = z->parent;
					map_left_rotate(m, z);
				}

				/* case 3 */
				z->parent->color = 0;
				z->parent->parent->color = 1;
				map_right_rotate(m, z->parent->parent);
			}
		} else {
			struct map_node *y = z->parent->parent->left;  // Uncle

			if (y != NULL && y->color == 1) {
				z->parent->color = 0;
				y->color = 0;
				z->parent->parent->color = 1;
				z = z->parent->parent;
			} else {
				if (z == z->parent->left) {
					z = z->parent;
					map_right_rotate(m, z);
				}

				z->parent->color = 0;
				z->parent->parent->color = 1;
				map_left_rotate(m, z->parent->parent);
			}
		}
	}

	/* case 0 */
	m->root->color = 0;
}

static int default_comparator(const void *key1, const void *key2,
			      size_t keylen1, size_t keylen2)
{
	return memcmp(key1, key2, keylen1 < keylen2 ? keylen1 : keylen2);
}

static void map_destroy_recursive(struct map *m, struct map_node *n)
{
	if (n != NULL) {
		map_destroy_recursive(m, n->left);
		map_destroy_recursive(m, n->right);

		free(n);
	}
}

int map_init(struct map *m, size_t value_size,
	     int (*cmp)(const void *, const void *, size_t, size_t))
{
	m->root = NULL;
	m->value_size = value_size;
	m->cmp = cmp == NULL ? default_comparator : cmp;
	m->hold_value = malloc(value_size);

	return m->hold_value == NULL ? 1 : 0;
}

void map_destroy(struct map *m)
{
	map_destroy_recursive(m, m->root);
	free(m->hold_value);
}

void *map_get2(struct map *m, const void *key, size_t keylen)
{
	struct map_node *cur = m->root;

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

int map_insert2(struct map *m,
		const void *key,
		size_t keylen,
		const void *value)
{
	struct map_node *n = map_new_node(key, keylen, value, m->value_size);

	if (n == NULL)
		return 1;

	if (m->root == NULL) {
		m->root = n;

		goto return_fixup;
	}

	struct map_node *p;
	struct map_node *cur = m->root;

	int cmp;

	while (cur != NULL) {
		p = cur;
		cur->size++;

		cmp = m->cmp(cur->kv, key, cur->keylen, keylen);

		if (cmp < 0)
			cur = cur->right;
		else if (cmp > 0)
			cur = cur->left;
		else
			assert(0 && "map already contains the element");
	}

	if (cmp < 0)
		p->right = n;
	else
		p->left = n;

	n->parent = p;

return_fixup:
	map_fixup(m, n);

	return 0;
}

struct map_entry map_select(struct map *m, size_t i)
{
	assert(i < map_size(m) && "map overflow");

	struct map_node *cur = m->root;

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

	return (struct map_entry) {
		.key = cur->kv,
		.keylen = cur->keylen,
		.value = &cur->kv[lf_map_align(cur->keylen)],
	};
}

size_t map_size(const struct map *m)
{
	return lf_map_node_size(m->root);
}

size_t map_rank(const struct map *m, const void *key, size_t keylen)
{
	struct map_node *cur = m->root;

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
