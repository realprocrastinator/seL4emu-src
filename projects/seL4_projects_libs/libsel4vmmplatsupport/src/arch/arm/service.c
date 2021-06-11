/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4/sel4.h>
#include <vka/object.h>
#include <vka/capops.h>

#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/arch/service.h>

int vmm_install_service(vm_t *vm, seL4_CPtr service, int index, uint32_t b)
{
    cspacepath_t src, dst;
    seL4_Word badge = b;
    int err;
    vka_cspace_make_path(vm->vka, service, &src);
    dst.root = vm->cspace.cspace_obj.cptr;
    dst.capPtr = index;
    dst.capDepth = VM_CSPACE_SIZE_BITS;
    err =  vka_cnode_mint(&dst, &src, seL4_AllRights, badge);
    return err;
}


