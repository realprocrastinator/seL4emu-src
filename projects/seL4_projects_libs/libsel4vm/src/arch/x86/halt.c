/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/*vm exits related with hlt'ing*/

#include <stdio.h>
#include <stdlib.h>

#include <sel4/sel4.h>

#include <sel4vm/guest_vm.h>

#include "vm.h"
#include "guest_state.h"
#include "processor/lapic.h"

/* Handling halt instruction VMExit Events. */
int vm_hlt_handler(vm_vcpu_t *vcpu)
{
    if (!(vm_guest_state_get_rflags(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr) & BIT(9))) {
        printf("vcpu %d is halted forever :(\n", vcpu->vcpu_id);
    }

    if (vm_apic_has_interrupt(vcpu) == -1) {
        /* Halted, don't reply until we get an interrupt */
        vcpu->vcpu_arch.guest_state->virt.interrupt_halt = 1;
    }

    vm_guest_exit_next_instruction(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
    return VM_EXIT_HANDLED;
}
