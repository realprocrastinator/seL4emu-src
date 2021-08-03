/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <mode/fastpath/fastpath.h>

#ifndef CONFIG_KERNEL_MCS
static inline int fastpath_reply_cap_check(cap_t cap)
{
    return cap_capType_equals(cap, cap_reply_cap);
}
#endif

void slowpath(syscall_t syscall)
#ifndef CONFIG_SEL4_USE_EMULATION
NORETURN
#endif
;

void fastpath_call(word_t cptr, word_t r_msgInfo)
#ifndef CONFIG_SEL4_USE_EMULATION
NORETURN
#endif
;
#ifdef CONFIG_KERNEL_MCS
void fastpath_reply_recv(word_t cptr, word_t r_msgInfo, word_t reply)
#else
void fastpath_reply_recv(word_t cptr, word_t r_msgInfo)
#endif
#ifndef CONFIG_SEL4_USE_EMULATION
NORETURN
#endif
;