/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/*vm exits related with vmx timer*/

#include <autoconf.h>
#include <sel4vm/gen_config.h>
#include <stdio.h>
#include <stdlib.h>

#include <sel4/sel4.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/arch/vmcs_fields.h>

#include "vm.h"
#include "vmcs.h"
#include "debug.h"

int vm_vmx_timer_handler(vm_vcpu_t *vcpu)
{
#ifdef CONFIG_LIB_VM_VMX_TIMER_DEBUG
    vm_print_guest_context(vcpu);
//    vm_vmcs_write(vmm->guest_vcpu, VMX_CONTROL_PIN_EXECUTION_CONTROLS, vm_vmcs_read(vmm->guest_vcpu, VMX_CONTROL_PIN_EXECUTION_CONTROLS) | BIT(6));
    int err = vm_vmcs_write(vcpu->vcpu.cptr, VMX_GUEST_VMX_PREEMPTION_TIMER_VALUE, CONFIG_LIB_VM_VMX_TIMER_TIMEOUT);
    if (err) {
        return VM_EXIT_HANDLE_ERROR;
    }
    return VM_EXIT_HANDLED;
#else
    return VM_EXIT_HANDLE_ERROR;
#endif
}
