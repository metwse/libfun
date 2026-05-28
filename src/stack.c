#ifndef LF_HEADERONLY
#include "util.h"
#include "../include/config.h"
#include "../include/stack.h"
#endif

#include <stddef.h>
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
	lf_unwrap(lf(stack_init)(s, item_size));
}

void lf(stack_destroy)(struct lf(stack) *s)
{
	free(s->data);
}

const void *lf(stack_pop)(struct lf(stack) *s)
{
	lf_assert(s->len, "stack underflow");

	void *item = lf(stack_at)(s, s->len - 1);

	s->len--;

	return item;
}

void *lf(stack_push)(struct lf(stack) *s, const void *item)
{
	if (s->len == s->cap) {
		s->cap *= 2;

		void *new_data = realloc(s->data, s->cap * s->item_size);

		if (new_data == NULL)
			return NULL;
		else
			s->data = new_data;

	}

	void *item_on_stack = &s->data[s->len * s->item_size];

	if (item != NULL)
		memcpy(item_on_stack, item, s->item_size);

	s->len++;

	return item_on_stack;
}

void *lf(stack_xpush)(struct lf(stack) *s, const void *item)
{
	void *push_res = lf(stack_push)(s, item);

	lf_assert(push_res != NULL, "insert returned NULL");

	return push_res;
}

void *lf(stack_top)(struct lf(stack) *s)
{
	lf_assert(s->len, "stack underflow");

	return lf(stack_at)(s, s->len - 1);
}

void *lf(stack_at)(struct lf(stack) *s, ptrdiff_t index)
{
	return &s->data[lfi(circular_index)(index, s->len) * s->item_size];
}

size_t lf(stack_len)(const struct lf(stack) *s)
{
	return s->len;
}
