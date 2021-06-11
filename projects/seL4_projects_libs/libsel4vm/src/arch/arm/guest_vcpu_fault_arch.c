/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4vm/guest_vcpu_fault.h>

#include "fault.h"

seL4_Word get_vcpu_fault_address(vm_vcpu_t *vcpu)
{
    fault_t *fault = vcpu->vcpu_arch.fault;
    return fault->addr;
}

seL4_Word get_vcpu_fault_ip(vm_vcpu_t *vcpu)
{
    fault_t *fault = vcpu->vcpu_arch.fault;
    return fault->ip;
}

seL4_Word get_vcpu_fault_data(vm_vcpu_t *vcpu)
{
    fault_t *fault = vcpu->vcpu_arch.fault;
    return fault_get_data(fault);
}

seL4_Word get_vcpu_fault_data_mask(vm_vcpu_t *vcpu)
{
    fault_t *fault = vcpu->vcpu_arch.fault;
    return fault_get_data_mask(fault);
}

size_t get_vcpu_fault_size(vm_vcpu_t *vcpu)
{
    fault_t *fault = vcpu->vcpu_arch.fault;
    return fault_get_width_size(fault);
}

bool is_vcpu_read_fault(vm_vcpu_t *vcpu)
{
    fault_t *fault = vcpu->vcpu_arch.fault;
    return fault_is_read(fault);
}

int set_vcpu_fault_data(vm_vcpu_t *vcpu, seL4_Word data)
{
    fault_t *fault = vcpu->vcpu_arch.fault;
    fault_set_data(fault, data);
    return 0;
}

void advance_vcpu_fault(vm_vcpu_t *vcpu)
{
    advance_fault(vcpu->vcpu_arch.fault);
    return;
}

void restart_vcpu_fault(vm_vcpu_t *vcpu)
{
    restart_fault(vcpu->vcpu_arch.fault);
    return;
}
