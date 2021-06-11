/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/* Debugging helper functions used by VMM lib.
 *     Authors:
 *         Qian Ge
 */

#include <stdio.h>
#include <stdlib.h>

#include <sel4/sel4.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/arch/guest_x86_context.h>

#include "debug.h"
#include "guest_state.h"
#include "vmcs.h"

/* Print out the context of a guest OS thread. */
void vm_print_guest_context(vm_vcpu_t *vcpu)
{
    unsigned int data_exit_info, data_exit_error;
    if (vm_vmcs_read(vcpu->vcpu.cptr, VMX_DATA_EXIT_INTERRUPT_INFO, &data_exit_info) ||
        vm_vmcs_read(vcpu->vcpu.cptr, VMX_DATA_EXIT_INTERRUPT_ERROR, &data_exit_error)) {
        return;
    }
    printf("================== GUEST OS CONTEXT =================\n");

    printf("exit info : reason 0x%x    qualification 0x%x   instruction len 0x%x interrupt info 0x%x interrupt error 0x%x\n",
           vm_guest_exit_get_reason(vcpu->vcpu_arch.guest_state), vm_guest_exit_get_qualification(vcpu->vcpu_arch.guest_state),
           vm_guest_exit_get_int_len(vcpu->vcpu_arch.guest_state), data_exit_info, data_exit_error);
    printf("            guest physical 0x%x     rflags 0x%x \n",
           vm_guest_exit_get_physical(vcpu->vcpu_arch.guest_state), vm_guest_state_get_rflags(vcpu->vcpu_arch.guest_state,
                                                                                              vcpu->vcpu.cptr));
    printf("            guest interruptibility 0x%x   control entry 0x%x\n",
           vm_guest_state_get_interruptibility(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr),
           vm_guest_state_get_control_entry(vcpu->vcpu_arch.guest_state));

    printf("eip 0x%8x\n",
           vm_guest_state_get_eip(vcpu->vcpu_arch.guest_state));
    unsigned int eax, ebx, ecx;
    vm_get_thread_context_reg(vcpu, VCPU_CONTEXT_EAX, &eax);
    vm_get_thread_context_reg(vcpu, VCPU_CONTEXT_EBX, &ebx);
    vm_get_thread_context_reg(vcpu, VCPU_CONTEXT_ECX, &ecx);
    printf("eax 0x%8x         ebx 0x%8x      ecx 0x%8x\n", eax, ebx, ecx);
    unsigned int edx, esi, edi;
    vm_get_thread_context_reg(vcpu, VCPU_CONTEXT_EDX, &edx);
    vm_get_thread_context_reg(vcpu, VCPU_CONTEXT_ESI, &esi);
    vm_get_thread_context_reg(vcpu, VCPU_CONTEXT_EDI, &edi);
    printf("edx 0x%8x         esi 0x%8x      edi 0x%8x\n", edx, esi, edi);
    unsigned int ebp;
    vm_get_thread_context_reg(vcpu, VCPU_CONTEXT_EBP, &ebp);
    printf("ebp 0x%8x\n", ebp);

    printf("cr0 0x%x      cr3 0x%x   cr4 0x%x\n", vm_guest_state_get_cr0(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr),
           vm_guest_state_get_cr3(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr),
           vm_guest_state_get_cr4(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr));
}
