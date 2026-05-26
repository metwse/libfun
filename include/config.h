/**
 * @file config.h
 * @brief Library-wide configuration.
 */

#ifndef LF_HASHMAP_INITIAL_CAP
/** @brief Initial capacity of the hashmap. */
#define LF_HASHMAP_INITIAL_CAP 64
#endif

#ifndef LF_STACK_INITIAL_CAP
/** @brief Initial capacity of the stack. */
#define LF_STACK_INITIAL_CAP 64
#endif

/** Public function prefixing */
#ifndef LIBFUN_PREFIX
/** @brief Function prefix. */
#define LIBFUN_PREFIX f
#endif

#ifndef lf
/** @cond */
#define lfi_concat_inner(prefix, name) prefix ## name
#define lfi_concat(prefix, name) lfi_concat_inner(prefix, name)
/** @endcond */

/* libfun public symbols. */
#define lf(name) lfi_concat(LIBFUN_PREFIX, name)
#endif


/** @cond */
#ifndef lfi_wur
#if defined(__GNUC__) || defined(__clang__)
#define lfi_wur __attribute__((warn_unused_result))
#else
#define lfi_wur
#endif
#endif

#ifndef lfi
/* Internal function declaration & name prefixing. */
#define lfi(name) lf(_libfun_internal_ ## name)

#define lfi_fdecl(ret_ty, name) static ret_ty lfi(name)
#endif
/** @endcond */
