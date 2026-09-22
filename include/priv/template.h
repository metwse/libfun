#ifndef LFI_TEMPLATE_H
#define LFI_TEMPLATE_H

#include "detail.h"  // IWYU pragma: export


/* check flag */
#define lfi_flag(flag) (flag & lfi_flags)

#if lfi_flag(fdebug_assertion)
#define lfi_debug_assertion(c, ...) lfi_assert(__VA_ARGS__)
#else
#define lfi_debug_assertion(c, ...) ((void) 0)
#endif

#endif
