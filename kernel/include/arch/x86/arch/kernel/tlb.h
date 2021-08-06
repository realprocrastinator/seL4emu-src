/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <arch/smp/ipi_inline.h>
#ifdef CONFIG_SEL4_USE_EMULATION
#include <emu/emu_assert.h>
#endif

static inline void invalidatePageStructureCacheASID(paddr_t root, asid_t asid, word_t mask)
{
    invalidateLocalPageStructureCacheASID(root, asid);
    SMP_COND_STATEMENT(doRemoteInvalidatePageStructureCacheASID(root, asid, mask));
}

static inline void invalidateTranslationSingle(vptr_t vptr, word_t mask)
{
    invalidateLocalTranslationSingle(vptr);
    SMP_COND_STATEMENT(doRemoteInvalidateTranslationSingle(vptr, mask));
}

static inline void invalidateTranslationSingleASID(vptr_t vptr, asid_t asid, word_t mask)
{
    invalidateLocalTranslationSingleASID(vptr, asid);
    SMP_COND_STATEMENT(doRemoteInvalidateTranslationSingleASID(vptr, asid, mask));
}

static inline void invalidateTranslationAll(word_t mask)
{
    invalidateLocalTranslationAll();
    SMP_COND_STATEMENT(doRemoteInvalidateTranslationAll(mask));
}


