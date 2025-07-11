#include "vector.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>


struct vec_result vec_init()
{
	return vec_with_capacity(0);
}

struct vec_result vec_with_capacity(size_t cap)
{
	struct vec_result r;

	struct vec *v = (struct vec *) malloc(sizeof(struct vec));
	void **arr = calloc(sizeof(void *), cap);

	if (arr == NULL || v == NULL) {
		r.v = VECE;
		r.d.err = VECE_MEM;

		return r;
	}

	v->_danger = arr;
	v->cap = cap;
	v->size = 0;

	r.v = VECOK_VEC;
	r.d.vec = v;

	return r;
}

void vec_destroy(struct vec *v)
{
	free(v->_danger);
	free(v);
}

struct vec_result vec_expand(struct vec *v, size_t cap)
{
	struct vec_result r;

	if (v->cap > cap) {
		r.v = VECE;
		r.d.err = VECE_CAP_IS_GREATER;
	} else if (v->cap == cap) {
		r.v = VECOK;
	} else {
		void **new_arr = realloc(v->_danger, cap * sizeof(void *));

		if (new_arr == NULL) {
			r.v = VECE;
			r.d.err = VECE_MEM;
		} else {
			r.v = VECOK;
			v->_danger = new_arr;
		}
	}

	return r;
}

struct vec_result vec_shrink(struct vec *v, size_t cap)
{
	struct vec_result r;
	r.v = VECOK_VEC;
	r.d.vec = NULL;

	if (v->cap < cap) {
		r.v = VECE;
		r.d.err = VECE_CAP_IS_LOWER;
		return r;
	} else if (v->cap > cap) {
		if (v->size <= cap)
			goto return_resize;

		void **stripped = calloc(v->size - cap, sizeof(void *));

		if (stripped == NULL)
			goto return_resize;

		for (size_t i = cap; i < v->size; v++)
			stripped[i - cap] = v->_danger[i];

		struct vec *stripped_vec = (struct vec *) malloc(sizeof(struct vec));
		stripped_vec->_danger = stripped;
		stripped_vec->cap = v->size - cap;
		stripped_vec->size = v->size - cap;

		r.d.vec = stripped_vec;
	}

return_resize:
	v->_danger = realloc(v->_danger, cap * sizeof(void *));
	v->cap = cap;
	v->size = cap;

	return r;
}

struct vec_result vec_shrink_to_fit(struct vec *v)
{
	return vec_shrink(v, v->size);
}

struct vec_result vec_append(struct vec *v, void *e)
{
	struct vec_result r;

	if (v->size == v->cap) {
		r = vec_expand(v, v->cap + VEC_EXPAND_DELTA);
		if (r.v == VECE)
			return r;
	} else {
		r.v = VECOK;
	}

	v->_danger[v->size] = e;
	v->size++;

	return r;
}

struct vec_result vec_insert(struct vec *v, size_t index, void *e)
{
	struct vec_result r;

	if (v->size < index) {
		r.v = VECE;
		r.d.err = VECE_OUT_OF_INDEX;
	} else if (v->size < v->cap) {
		for (int i = v->size; i > index; i--) {
			v->_danger[i] = v->_danger[i - 1];
		}
		v->_danger[index] = e;

		r.v = VECOK;
	} else {
		r.v = VECE;
		r.d.err = VECE_INSUFFICIENT_CAP;
	}

	return r;
}

struct vec_result vec_get(const struct vec *v, size_t index)
{
	struct vec_result r;

	if (v->size <= index) {
		r.v = VECE;
		r.d.err = VECE_OUT_OF_INDEX;
	} else {
		r.v = VECOK_ELEMENT;
		r.d.element = &v->_danger[index];
	}

	return r;
}

struct vec_result vec_replace(struct vec *v, size_t index, void *e)
{
	struct vec_result r;

	if (v->size <= index) {
		r.v = VECE;
		r.d.err = VECE_OUT_OF_INDEX;
	} else {
		r.v = VECOK_ELEMENT;
		r.d.element = &v->_danger[index];
		v->_danger[index] = e;
	}

	return r;
}

struct vec_result vec_remove(struct vec *v, size_t index)
{
	struct vec_result r;

	if (v->size <= index) {
		r.v = VECE;
		r.d.err = VECE_OUT_OF_INDEX;
	} else {
		r.v = VECOK_ELEMENT;
		r.d.element = &v->_danger[index];

		for (int i = index; i < v->size - 1; i++) {
			v->_danger[i] = v->_danger[i + 1];
		}

		v->size--;
	}

	return r;
}

struct vec_result vec_pop(struct vec *v)
{
	struct vec_result r;

	if (v->size == 0) {
		r.v = VECE;
		r.d.err = VECE_OUT_OF_INDEX;
	} else {
		v->size--;

		r.v = VECOK_ELEMENT;
		r.d.element = &v->_danger[v->size];

	}

	return r;
}
