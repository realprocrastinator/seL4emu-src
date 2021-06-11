/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <sel4vm/guest_vm.h>

/*typedef int(*vmexit_handler_ptr)(vm_vcpu_t *vcpu);*/

/*vm exit handlers*/
int vm_cpuid_handler(vm_vcpu_t *vcpu);
int vm_ept_violation_handler(vm_vcpu_t *vcpu);
int vm_wrmsr_handler(vm_vcpu_t *vcpu);
int vm_rdmsr_handler(vm_vcpu_t *vcpu);
int vm_io_instruction_handler(vm_vcpu_t *vcpu);
int vm_hlt_handler(vm_vcpu_t *vcpu);
int vm_vmx_timer_handler(vm_vcpu_t *vcpu);
int vm_cr_access_handler(vm_vcpu_t *vcpu);
int vm_vmcall_handler(vm_vcpu_t *vcpu);
int vm_pending_interrupt_handler(vm_vcpu_t *vcpu);
