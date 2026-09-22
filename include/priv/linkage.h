/** Define T and lfi_ctype macros before including this file.
 *
 * T: <container_name> ...container args
 * lfi_ctype: type of the container (vec, hashmap etc.)
 * lfi_name: name of the container
 */

#include "../common.h"

#include "detail.h"

/* container name mangling */
#define lfi_self_name \
	lf(lfi_concat(lfi_ctype, lfi_concat(_, lfi_name)))

#define lfi_self struct lfi_self_name


/* member function name mangling */
#define lfi_memb(name) \
	lfi_concat(lfi_self_name, lfi_concat(_, name))

/* internal identifiers */
#define lfi(name) lfi_concat(lfi_self_name, lfi_g(name))
