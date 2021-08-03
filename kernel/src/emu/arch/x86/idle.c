/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <config.h>
#include <api/debug.h>

void idle_thread(void)
{
    while (1) {
      // we are in the user space can't use `hlt` instruction, lets just loop forever!
      // asm volatile("hlt");
    }
}

void halt(void)
{
    /* halt is actually, idle thread without the interrupts, but in userspace we can't diable interrupt */
    // asm volatile("cli");

#ifdef CONFIG_PRINTING
    printf("halting...");
#ifdef CONFIG_DEBUG_BUILD
    // debug_printKernelEntryReason();
#endif
#endif
    idle_thread();
    UNREACHABLE();
}
