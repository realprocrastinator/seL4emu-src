/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <arch/kernel/thread.h>
#include <kernel/thread.h>

void Arch_postModifyRegisters(tcb_t *tptr) { Mode_postModifyRegisters(tptr); }
