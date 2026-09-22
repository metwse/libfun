#ifndef LFI_DETAIL_H
#define LFI_DETAIL_H


#include <stdlib.h>  // IWYU pragma: export

#if (defined(__unix__) || defined(__APPLE__) || defined(__linux__)) && \
    (defined(__GNUC__) || defined(__clang__))

#include <stdio.h>  // IWYU pragma: begin_exports
#include <signal.h>  // IWYU pragma: end_exports

#define lfi_assert_stringify_detail(a) #a
#define lfi_assert_stringify(a) lfi_assert_stringify_detail(a)
#define lfi_assert(c, ...) do { \
		if (!(c)) { \
			fprintf(stderr, "["  __FILE__ ":" \
				lfi_assert_stringify(__LINE__) "] " \
				"Assertion failed for: " \
				lfi_assert_stringify(c) \
				"\n> " __VA_ARGS__); \
			fputc('\n', stderr); \
			raise(SIGINT); \
		} \
	} while(0)

#else

#include <assert.h>  // IWYU pragma: export

#define lfi_assert(c, ...) do { assert(c); abort(); } while (0)

#endif

#define lfi_unwrap(c) lfi_assert(c, "discarded result indicate error")

#define lfi_unreachable do { lfi_assert(0, "unreachable"); abort(); } while (0)


#if defined(__GNUC__) || defined(__clang__)
#define lfi_wur __attribute__((warn_unused_result))
#else
#define lfi_wur
#endif


#define lfi_concat_inner(prefix, name) prefix ## name
#define lfi_concat(prefix, name) lfi_concat_inner(prefix, name)

#define lfi_after_arg1_inner(_1, ...) __VA_ARGS__
#define lfi_after_arg1(...) lfi_after_arg1_inner(__VA_ARGS__,)
#define lfi_after_arg2(...) lfi_after_arg1(lfi_after_arg1(__VA_ARGS__,))
#define lfi_after_arg4(...) lfi_after_arg2(lfi_after_arg2(__VA_ARGS__,))

#define lfi_arg1_inner(arg, ...) arg
#define lfi_arg1(...) lfi_arg1_inner(__VA_ARGS__,)

#define lfi_arg2(...) lfi_arg1(lfi_after_arg1(__VA_ARGS__,))
#define lfi_arg3(...) lfi_arg1(lfi_after_arg2(__VA_ARGS__,))
#define lfi_arg4(...) lfi_arg1(lfi_after_arg2(lfi_after_arg1(__VA_ARGS__,)))
#define lfi_arg5(...) lfi_arg1(lfi_after_arg4(__VA_ARGS__,))
#define lfi_arg6(...) lfi_arg1(lfi_after_arg4(lfi_after_arg1(__VA_ARGS__,)))
#define lfi_arg7(...) lfi_arg1(lfi_after_arg4(lfi_after_arg2(__VA_ARGS__,)))
#define lfi_arg8(...) lfi_arg1(lfi_after_arg4(lfi_after_arg2(lfi_after_arg1(__VA_ARGS__,))))

#define lfi_g(name) lfi_concat(_libfun_internal_, name)


#endif
