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

#ifndef LF_STACK_H
#define LF_STACK_H

#include <stddef.h>
#include <stdlib.h>

#define LF_STACK_INITIAL_CAP 64

/** @cond */
#include "priv/detail.h"
#include "priv/template.h"

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


/* @cond */
static inline int lfi(resize)(lfi_self *s, size_t new_cap)
{
	if (s->lfi(cap) == new_cap)
		return 0;

	lfi_key *old_data = s->lfi(data);

	s->lfi(data) = (lfi_key *) realloc(s->lfi(data),
					   new_cap * sizeof(lfi_key));

	if (s->lfi(data) == NULL) {
		s->lfi(data) = old_data;
		return 1;
	}

	s->lfi(cap) = new_cap;
	return 0;
}
/* @endcond */


/** @brief Creates a new stack, returns non-zero if a memory allocation failure
 * occurs. */
lfi_wur static inline int lfi_memb(with_cap)(lfi_self *s, size_t cap)
{
	if (cap == 0)
		cap = 1;

	s->lfi(cap) = cap;
	s->lfi(len) = 0;
	s->lfi(data) = malloc(sizeof(lfi_key) * cap);

	return s->lfi(data) == NULL ? 1 : 0;
}

/** @brief Identical to stack_with_cap(), but raises an error if memory allocation
 * fails. */
static inline void lfi_memb(xwith_cap)(lfi_self *s, size_t cap)
{
	lfi_unwrap(lfi_memb(with_cap)(s, cap) == 0);
}

/** @brief Creates a new stack, returns non-zero if a memory allocation failure
 * occurs. */
lfi_wur static inline int lfi_memb(init)(lfi_self *s)
{
	return lfi_memb(with_cap(s, LF_STACK_INITIAL_CAP));
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

/** @brief Removes count element and returns array of removed elements. */
static inline lfi_key const *lfi_memb(multipop)(lfi_self *s, size_t count)
{
	lfi_debug_assertion(s->lfi(len) >= count, "stack underflow");
	lfi_debug_assertion(count > 0, "cannot pop 0 elements");

	s->lfi(len) -= count;

	return &s->lfi(data)[s->lfi(len)];
}

/** @brief Removes and returns the top element from the stack. */
static inline lfi_key const *lfi_memb(pop)(lfi_self *s)
{
	lfi_debug_assertion(s->lfi(len) > 0, "stack underflow");

	return &s->lfi(data)[--s->lfi(len)];
}

/** @brief Pushes multiple elements to top of the stack. */
lfi_wur static inline lfi_key *lfi_memb(multipush)(lfi_self *s,
					           lfi_key const items[],
					           size_t count)
{
	while (s->lfi(len) + count > s->lfi(cap))
		lfi(resize)(s, s->lfi(cap) * 2);

	if (s->lfi(len) + count > s->lfi(cap))
		return NULL;

	lfi_key *pushed_bottom = &s->lfi(data)[s->lfi(len)];

	if (items != NULL) {
		for (size_t i = 0; i < count; i++)
			s->lfi(data)[s->lfi(len)++] = items[i];
	} else {
		s->lfi(len) += count;
	}

	return pushed_bottom;
}

/** @brief Identical to stack_push(), but raises an error if memory allocation
 * fails. */
static inline lfi_key *lfi_memb(xmultipush)(lfi_self *s,
					    lfi_key const items[],
					    size_t count)
{
	lfi_key *pushed_bottom = lfi_memb(multipush)(s, items, count);

	lfi_unwrap(pushed_bottom != NULL);

	return pushed_bottom;
}

/** @brief Pushes an element to top of the stack. */
lfi_wur static inline lfi_key *lfi_memb(push)(lfi_self *s, lfi_key const *item)
{
	return lfi_memb(multipush)(s, item, 1);
}

/** @brief Identical to stack_push(), but raises an error if memory allocation
 * fails. */
static inline lfi_key *lfi_memb(xpush)(lfi_self *s, lfi_key const *item)
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

/** @brief Returns the stack capacity. */
static inline size_t lfi_memb(cap)(const lfi_self *s)
{
	return s->lfi(cap);
}

/** @brief Clears the stack. */
static inline void lfi_memb(clear)(lfi_self *s)
{
	s->lfi(len) = 0;
}

/** @brief Shrinks the stack to given capacity.
 *
 * If a capacity lower than the number of stored elements is specified, the
 * capacity is reduced to the number of stored elements.
 *
 * See stack_shrink_to_fit()
 *
 * Returns non-zero if a memory allocation failure occurs. */
static inline int lfi_memb(shrink_to)(lfi_self *s, size_t new_cap)
{
	if (new_cap < s->lfi(len))
		return lfi(resize)(s, s->lfi(len));
	else if (new_cap == 0)
		return lfi(resize)(s, 1);
	else
		return lfi(resize)(s, new_cap);
}

/** @brief Shrinks the stack as much as possible.
 *
 * @note This operation is best-effort shrink, the capacity may left as-is
 *       if a memory allocation error occur.
 *
 * Returns non-zero if a memory allocation failure occurs. */
static inline int lfi_memb(shrink_to_fit)(lfi_self *s)
{
	return lfi_memb(shrink_to)(s, s->lfi(len));
}

/** @brief Reserves capacity for at least `additional` more elements to be
 * inserted in the stack.
 *
 * @note This operation is best-effort grow, the capacity may left as-is if a
 *       memory allocation failure occur.
 *
 * Returns non-zero if a memory allocation failure occurs. */
static inline int lfi_memb(reserve)(lfi_self *s, size_t additional)
{
	if (additional == 0)
		return 0;

	size_t new_cap = s->lfi(len) + additional;

	if (new_cap > s->lfi(cap))
		return lfi(resize)(s, new_cap);
	else
		return 0;
}


/* @cond */
#include "priv/finalize.h"
/* @endcond */
