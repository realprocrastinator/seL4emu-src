/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4utils/vspace.h>
#include <sel4utils/vspace_internal.h>

static inline int guest_vspace_map_page_arch(vspace_t *vspace, seL4_CPtr cap, void *vaddr, seL4_CapRights_t rights,
                                             int cacheable, size_t size_bits)
{
    return sel4utils_map_page_pd(vspace, cap, vaddr, rights, cacheable, size_bits);
}
