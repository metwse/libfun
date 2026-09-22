#ifndef LF_COMMON_H
#define LF_COMMON_H


#include "priv/detail.h"


/** @brief Library prefix, defaults to `f`. */
#ifndef LIBFUN_PREFIX
#define LIBFUN_PREFIX f
#endif

#define lf(name) lfi_concat(LIBFUN_PREFIX, name)


#endif
