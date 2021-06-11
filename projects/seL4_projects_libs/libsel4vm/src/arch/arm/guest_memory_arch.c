/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vm_util.h>
#include <sel4vm/guest_memory.h>

uintptr_t vm_arm_ipa_to_pa(vm_t *vm, uintptr_t ipa_base, size_t size)
{
    seL4_ARM_Page_GetAddress_t ret;
    uintptr_t pa_base = 0;
    uintptr_t ipa;
    vspace_t *vspace;
    vspace = vm_get_vspace(vm);
    ipa = ipa_base;
    do {
        seL4_CPtr cap;
        int bits;
        /* Find the cap */
        cap = vspace_get_cap(vspace, (void *)ipa);
        if (cap == seL4_CapNull) {
            return 0;
        }
        /* Find mapping size */
        bits = vspace_get_cookie(vspace, (void *)ipa);
        assert(bits == 12 || bits == 21);
        /* Find the physical address */
        ret = seL4_ARM_Page_GetAddress(cap);
        if (ret.error) {
            return 0;
        }
        if (ipa == ipa_base) {
            /* Record the result */
            pa_base = ret.paddr + (ipa & MASK(bits));
            /* From here on, ipa and ret.paddr will be aligned */
            ipa &= ~MASK(bits);
        } else {
            /* Check for a contiguous mapping */
            if (ret.paddr - pa_base != ipa - ipa_base) {
                return 0;
            }
        }
        ipa += BIT(bits);
    } while (ipa - ipa_base < size);
    return pa_base;
}
