/*
 * Copyright 2016, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <config.h>
#include <util.h>
#ifdef CONFIG_SEL4_USE_EMULATION
#include <arch/machine.h> // for prototype of x86_load_fsgs_base
#include <arch/api/syscall.h> // for syscall_t
#endif

static inline void arch_c_entry_hook(void)
{
#ifdef CONFIG_FSGSBASE_INST
    tcb_t *tcb = NODE_STATE(ksCurThread);
    x86_save_fsgs_base(tcb, SMP_TERNARY(getCurrentCPUIndex(), 0));
#endif
}

static inline void arch_c_exit_hook(void)
{
    /* Restore the values ofthe FS and GS base. */
    tcb_t *tcb = NODE_STATE(ksCurThread);
    x86_load_fsgs_base(tcb,  SMP_TERNARY(getCurrentCPUIndex(), 0));
}

#ifdef CONFIG_KERNEL_MCS
void c_handle_syscall(word_t cptr, word_t msgInfo, syscall_t syscall, word_t reply)
#else
void c_handle_syscall(word_t cptr, word_t msgInfo, syscall_t syscall)
#endif
#ifndef CONFIG_SEL4_USE_EMULATION
VISIBLE NORETURN
#endif
;

void restore_user_context(void)
#ifndef CONFIG_SEL4_USE_EMULATION
VISIBLE NORETURN
#endif
;

void c_nested_interrupt(int irq)
#ifndef CONFIG_SEL4_USE_EMULATION
VISIBLE
#endif
;

void c_handle_interrupt(int irq, int syscall)
#ifndef CONFIG_SEL4_USE_EMULATION
VISIBLE NORETURN
#endif
;

void c_handle_vmexit(void)
#ifndef CONFIG_SEL4_USE_EMULATION
VISIBLE NORETURN
#endif
;
