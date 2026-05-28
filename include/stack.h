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
#include "common.h"
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
void lf(stack_xinit)(struct lf(stack) *stack, size_t value_size);

/** @brief Clears the memory allocated by the stack. */
void lf(stack_destroy)(struct lf(stack) *stack);

/** @brief Removes and returns the top element from the stack. */
const void *lf(stack_pop)(struct lf(stack) *stack);

/** @brief Pushes an element to the top of the stack. */
void *lf(stack_push)(struct lf(stack) *stack, const void *item) lfi_wur;

/** @brief Identical to stack_push(), but raises an error if memory allocation
 * fails. */
void *lf(stack_xpush)(struct lf(stack) *stack, const void *item);

/** @brief Returns the top element of the stack. */
void *lf(stack_top)(struct lf(stack) *stack);

/** @brief Returns the element at the specified `index`. */
void *lf(stack_at)(struct lf(stack) *stack, ptrdiff_t index);

/** @brief Returns the total number of elements. */
size_t lf(stack_len)(const struct lf(stack) *stack);


#endif
