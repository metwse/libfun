/*!
 * Dynamicly resizing array, aka. vector.
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include <stddef.h>
#include <stdbool.h>

#ifndef VEC_EXPAND_DELTA
#define VEC_EXPAND_DELTA 16
#endif // VECTOR_EXPAND_DELTA


/**
 * Dynamic array implementation.
 */
struct vec {
	// Pointer to the array data. **DO NOT USE DIRECTLY.**
	void **_danger;
	// Number of elements currently stored.
	size_t size;
	// Current capacity of the array.
	size_t cap;
};

/**
 * Vector error types.
 */
enum vec_error {
	// Memory allocation error.
	VECE_MEM,
	// Tried to expand the array to a smaller capacity than it already has.
	VECE_CAP_IS_GREATER,
	// Tried to shrink the array to a smaller capacity than it already has.
	VECE_CAP_IS_LOWER,
	// Size exceeded.
	VECE_OUT_OF_INDEX,
	// Size exceeded.
	VECE_INSUFFICIENT_CAP,
};

/**
 * Holds different kinds of result data for vector operations.
 */
union vec_result_data {
	// Error type when operation fails.
	enum vec_error err;
	// Resulting vector on successful operation.
	struct vec *vec;
	// Pointer to underlying data returned by pop, remove, get, etc.
	void *element;
};

/**
 * Indicates the type of result returned by a vector operation.
 */
enum vec_result_variant {
	// Indicates an error occurred.
	VECE,
	// Indicates success with no additional data.
	VECOK,
	// Indicates success with a returned vector.
	VECOK_VEC,
	// Pointer to an element.
	VECOK_ELEMENT,
};

/**
 * Result of a vector operation.
 */
struct vec_result {
	// Type of result.
	enum vec_result_variant v;
	// Associated data for the result.
	union vec_result_data d;
};

/**
 * Creates a new empty vector.
 *
 * @return A {@code vec_result} with the {@code VECOK_VEC} variant if
 *         successful.
 */
struct vec_result vec_init();

/**
 * Creates a new empty vector with capacity.
 *
 * @return A {@code vec_result} with the {@code VECOK_VEC} variant if
 *         successful.
 */
struct vec_result vec_with_capacity(size_t cap);

/**
 * Frees the allocated memory for the vector.
 */
void vec_destroy(struct vec *v);

/**
 * Expands the capacity of the vector.
 *
 * @return A {@code vec_result} with the {@code VECOK} variant if successful,
 *         or {@code VECE} with an appropriate error if failed
 */
struct vec_result vec_expand(struct vec *v, size_t cap);

/**
 * Shrinks the capacity of the vector to given capacity.
 *
 * @return A {@code vec_result} with the {@code VECOK_VEC} variant containing
 *         the stripped vector part if successful.
 */
struct vec_result vec_shrink(struct vec *v, size_t cap);

/**
 * Shrinks the capacity of the vector to fit its current size.
 *
 * @return A {@code vec_result} with the {@code VECOK} variant containing
 *         the stripped vector part if successful.
 */
struct vec_result vec_shrink_to_fit(struct vec *v);

/**
 * Adds an element to the vector. Expands the vector if necessary.
 *
 * @return A {@code vec_result} with the {@code VECOK} variant if successful.
 */
struct vec_result vec_append(struct vec *v, void *e);

/**
 * Adds an element to the vector at given index.
 *
 * @return A {@code vec_result} with the {@code VECOK} variant if successful.
 */
struct vec_result vec_insert(struct vec *v, size_t index, void *e);

/**
 * Gets the element at given index.
 *
 * @return A {@code vec_result} with the {@code VECOK_ELEMENT} variant if
 *         successful.
 */
struct vec_result vec_get(const struct vec *v, size_t index);

/**
 * Replaces the element at given index.
 *
 * @return A {@code vec_result} with the {@code VECOK_ELEMENT} variant if
 *         successful.
 */
struct vec_result vec_replace(struct vec *v, size_t index, void *e);

/**
 * Removes the element at given index from vector.
 *
 * @return A {@code vec_result} with the {@code VECOK_ELEMENT} variant if
 *         successful.
 */
struct vec_result vec_remove(struct vec *v, size_t index);

/**
 * Removes the element at last index of vector.
 *
 * @return A {@code vec_result} with the {@code VECOK_ELEMENT} variant if
 *         successful.
 */
struct vec_result vec_pop(struct vec *v);


#endif // VECTOR_H_
