#define __NEED_size_t
#define __NEED_uintptr_t
#include "bits/mini_alltypes.h"

size_t mini_strlen (const char *);
char *mini_strerror (int);

/* Copy N bytes of SRC to DEST. As we don't need to modify this part,
 * we just reference to the global definition.
 */
extern void *memcpy(void *restrict dest, const void *restrict src, size_t n);