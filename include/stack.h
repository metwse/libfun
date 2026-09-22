/**
 * @file stack.h
 * @brief Basic stack.
 *
 * ```
 * #define T <key-type>, <name>[, (<flags>)]
 * ```
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
 *
 * @note References to elements in a stack are valid until the next reisizing
 *       operation. E.g. if you push an element, previous pointers returned
 *       by get operations are invalidated.
 */

#ifndef LFI_DOXYGEN

/** @cond */
#ifndef LF_STACK_H
#define LF_STACK_H

#include "priv/detail.h"
#include "priv/template.h"

#include <stddef.h>
#include <stdlib.h>

#define LFI_STACK_INITIAL_CAP 64

#endif  // LF_STACK_H

#ifndef T
#define T int, int
#endif

#define lfi_ctype stack
#define lfi_name lfi_arg2(T)
#define lfi_flags lfi_arg3(T, 0, 0)

#include "priv/linkage.h"

#define lfi_key lfi_arg1(T)

/** @endcond */
#else  // LFI LFI_DOXYGEN

#define lfi_self struct fstack
#define lfi_key type
#define lfi_memb(name) fstack_ ## name

#endif


/** @brief stack. */
lfi_self {
    /** @cond */
    lfi_key *lfi(data);
    size_t lfi(cap);
    size_t lfi(len);
    /** @endcond */
};


/** @brief Creates a new stack, returns non-zero if a memory allocation failure
 * occurs. */
lfi_wur static inline int lfi_memb(init)(lfi_self *s)
{
	s->lfi(cap) = LFI_STACK_INITIAL_CAP;
	s->lfi(len) = 0;
	s->lfi(data) =
		(lfi_key *) malloc(sizeof(lfi_key) * LFI_STACK_INITIAL_CAP);

	return s->lfi(data) == NULL ? 1 : 0;
}

/** @brief Identical to stack_init(), but raises an error if memory allocation
 * fails. */
static inline void lfi_memb(xinit)(lfi_self *s)
{
	lfi_unwrap(lfi_memb(init)(s) == 0);
}

/** @brief Clears the memory allocated by the stack. */
static inline void lfi_memb(destroy)(lfi_self *s)
{
	free(s->lfi(data));
}

/** @brief Removes and returns the top element from the stack. */
static inline lfi_key const *lfi_memb(pop)(lfi_self *s)
{
	lfi_debug_assertion(s->lfi(len) > 0, "stack underflow");

	return &s->lfi(data)[--s->lfi(len)];
}

/** @brief Pushes an element to the top of the stack. */
lfi_wur static inline lfi_key *lfi_memb(push)(lfi_self *s,
					       lfi_key const *item)
{
	if (s->lfi(len) == s->lfi(cap)) {
		s->lfi(cap) *= 2;

		lfi_key *new_data = (lfi_key *) realloc(s->lfi(data),
							s->lfi(cap) * sizeof(lfi_key));

		if (new_data == NULL)
			return NULL;
		else
			s->lfi(data) = new_data;
	}

	lfi_key *item_on_stack = &s->lfi(data)[s->lfi(len)++];
	if (item != NULL)
		*item_on_stack = *item;

	return item_on_stack;
}

/** @brief Identical to stack_push(), but raises an error if memory allocation
 * fails. */
static inline lfi_key *lfi_memb(xpush)(lfi_self *s,
					lfi_key const *item)
{
	lfi_key *item_on_stack = lfi_memb(push)(s, item);

	lfi_unwrap(item_on_stack != NULL);

	return item_on_stack;
}

/** @brief Returns the top element of the stack. */
static inline lfi_key const *lfi_memb(top)(const lfi_self *s)
{
	lfi_debug_assertion(s->lfi(len) > 0, "stack underflow");

	return &s->lfi(data)[s->lfi(len) - 1];
}

/** @brief Returns the element at the specified `index`. */
static inline lfi_key const *lfi_memb(at)(const lfi_self *s, size_t index)
{
	lfi_debug_assertion(s->lfi(len) > index, "out of bounds");

	return &s->lfi(data)[index];
}

/** @brief Returns the nth element from top. */
static inline lfi_key const *lfi_memb(peek)(const lfi_self *s, size_t distance)
{
	lfi_debug_assertion(s->lfi(len) > distance, "out of bounds");

	return &s->lfi(data)[s->lfi(len) - distance - 1];
}

/** @brief See stack_top_mut(). */
static inline lfi_key *lfi_memb(top_mut)(lfi_self *s)
{
	return (lfi_key *) lfi_memb(top)(s);
}

/** @brief See stack_at_mut(). */
static inline lfi_key *lfi_memb(at_mut)(lfi_self *s, size_t index)
{
	return (lfi_key *) lfi_memb(at)(s, index);
}

/** @brief See stack_peek_mut(). */
static inline lfi_key *lfi_memb(peek_mut)(lfi_self *s, size_t distance)
{
	return (lfi_key *) lfi_memb(peek)(s, distance);
}

/** @brief Returns the total number of elements. */
static inline size_t lfi_memb(len)(const lfi_self *s)
{
	return s->lfi(len);
}


#include "priv/finalize.h"
