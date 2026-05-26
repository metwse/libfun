#ifndef LF_COMMON_H

#include <stdlib.h>  // IWYU pragma: export


/** @brief Assertion macro that prints formatted error message if failed. */
#if (defined(__unix__) || defined(__APPLE__) || defined(__linux__)) && \
    (defined(__GNUC__) || defined(__clang__))

#include <stdio.h>  // IWYU pragma: begin_exports
#include <signal.h>  // IWYU pragma: end_exports

/** @cond */
#define lf_assert_stringify_detail(a) #a
#define lf_assert_stringify(a) lf_assert_stringify_detail(a)
/** @endcond */

#define lf_assert(c, ...) do { \
		if (!(c)) { \
			fprintf(stderr, "["  __FILE__ ":" \
				lf_assert_stringify(__LINE__) "] " \
				"Assertion failed for: " \
				lf_assert_stringify(c) \
				"\n> " __VA_ARGS__); \
			fputc('\n', stderr); \
			raise(SIGINT); \
		} \
	} while(0)

#else

#include <assert.h>  // IWYU pragma: export

#define lf_assert(c, ...) do { assert(c); abort(); } while (0)

#endif

/** @brief Counterpart of the assert(), expects the condition to be false. */
#define lf_unwrap(c) lf_assert(!(c), "discarded result indicate error")

/** @brief Unreachable assertion. */
#define lf_unreachable(c) do { lf_assert(0, "unreachable"); abort(); } while (0)


#endif
