#include "../../include/vector.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>


struct vec_result vec_init()
{
	return vec_with_capacity(VEC_EXPAND_DELTA);
}

struct vec_result vec_with_capacity(size_t cap)
{
	struct vec *v = (struct vec *) malloc(sizeof(struct vec));

	if (v == NULL)
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_NOMEM
		};

	void **arr;
	if (cap) {
		arr = malloc(sizeof(void *) * cap);

		if (arr == NULL)
			return (struct vec_result) {
				.v = VECE,
				.d.err = VECE_NOMEM
			};
	} else {
		arr = NULL;
	}

	v->_danger = arr;
	v->cap = cap;
	v->size = 0;

	return (struct vec_result) { .v = VECOK_VEC, .d.vec = v };
}

void vec_destroy(struct vec *v)
{
	if (v->_danger)
		free(v->_danger);
	free(v);
}

struct vec_result vec_expand(struct vec *v, size_t cap)
{
	if (v->cap > cap)
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_CAP_IS_GREATER
		};
	else if (v->cap == cap)
		return (struct vec_result) { .v = VECOK };

	void **new_arr;
	if (v->_danger)
		new_arr = realloc(v->_danger, cap * sizeof(void *));
	else
		new_arr = malloc(cap * sizeof(void *));

	if (new_arr == NULL)
		return (struct vec_result) { .v = VECE, .d.err = VECE_NOMEM };

	v->_danger = new_arr;
	v->cap = cap;

	return (struct vec_result) { .v = VECOK };
}

struct vec_result vec_shrink(struct vec *v, size_t cap)
{
	struct vec *stripped_vec = NULL;
	void **new_arr;

	if (v->cap < cap) {
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_CAP_IS_LOWER
		};
	}
	if (v->cap > cap) {
		if (v->size <= cap)
			goto return_resize;

		void **stripped = malloc((v->size - cap) * sizeof(void *));

		if (stripped == NULL)
			goto return_resize;

		memcpy(
			stripped,
			&v->_danger[cap], (v->size - cap) * sizeof(void *)
		);

		stripped_vec = (struct vec *) malloc(sizeof(struct vec));

		stripped_vec->_danger = stripped;
		stripped_vec->cap = v->size - cap;
		stripped_vec->size = v->size - cap;
	}

return_resize:
	if (cap) {
		new_arr = realloc(v->_danger, cap * sizeof(void *));
		if (new_arr == NULL) {
			if (stripped_vec)
				vec_destroy(stripped_vec);

			return (struct vec_result) {
				.v = VECE,
				.d.err= VECE_NOMEM
			};
		}

		v->_danger = new_arr;
	} else {
		free(v->_danger);
		v->_danger = NULL;
	}

	v->cap = cap;
	if (cap < v->size)
		v->size = cap;

	return (struct vec_result) { .v = VECOK_VEC, .d.vec = stripped_vec };
}

inline struct vec_result vec_shrink_to_fit(struct vec *v)
{
	return vec_shrink(v, v->size);
}

struct vec_result vec_insert(struct vec *v, size_t index, void *e)
{
	if (v->size < index)
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_OUT_OF_BOUNDS
		};

	if (v->size == v->cap) {
		struct vec_result r = vec_expand(v, v->cap + VEC_EXPAND_DELTA);
		if (r.v == VECE)
			return r;
	}

	for (int i = v->size; i > index; i--)
		v->_danger[i] = v->_danger[i - 1];

	v->_danger[index] = e;
	v->size++;

	return (struct vec_result) { .v = VECOK };
}

inline struct vec_result vec_append(struct vec *v, void *e)
{
	return vec_insert(v, v->size, e);
}


struct vec_result vec_get(const struct vec *v, size_t index)
{
	if (v->size <= index)
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_OUT_OF_BOUNDS
		};

	return (struct vec_result) {
		.v = VECOK_ELEMENT,
		.d.element = v->_danger[index]
	};
}

struct vec_result vec_replace(struct vec *v, size_t index, void *e)
{
	void *hold;

	if (v->size <= index)
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_OUT_OF_BOUNDS
		};

	hold = v->_danger[index];
	v->_danger[index] = e;

	return (struct vec_result) { .v = VECOK_ELEMENT, .d.element = hold };
}

struct vec_result vec_remove(struct vec *v, size_t index)
{
	void *hold;

	if (v->size <= index)
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_OUT_OF_BOUNDS
		};
	if (v->size == 0)
		return (struct vec_result) {
			.v = VECE,
			.d.err = VECE_EMPTY
		};

	hold = v->_danger[index];

	for (int i = index; i < v->size - 1; i++)
		v->_danger[i] = v->_danger[i + 1];

	if (v->size == 0) {
		free(v->_danger);
		v->_danger = NULL;
	}

	v->size--;
	return (struct vec_result) { .v = VECOK_ELEMENT, .d.element = hold };
}

inline struct vec_result vec_pop(struct vec *v)
{
	return vec_remove(v, v->size - 1);
}
