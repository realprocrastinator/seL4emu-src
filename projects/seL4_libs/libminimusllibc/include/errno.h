#ifndef	_ERRNO_H
#define _ERRNO_H

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "features.h"

#include "bits/errno.h"

// #ifdef __GNUC__
// __attribute__((const))
// #endif
// int *__mini_errno_location(void);
// #define errno (*__mini_errno_location())
int mini_errno;

// #ifdef _GNU_SOURCE
// extern char *program_invocation_short_name, *program_invocation_name;
// #endif

// #ifdef __cplusplus
// }
// #endif

#endif