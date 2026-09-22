#ifndef LFI_TEMPLATE_H
#define LFI_TEMPLATE_H


/* check flag */
#define lfi_flag(flag) (flag & lfi_flags)

#define lfi_remove_paren_inner(...) __VA_ARGS__
#define lfi_remove_paren(...) lfi_remove_paren_inner __VA_ARGS__


#endif
