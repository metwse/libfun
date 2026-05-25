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
