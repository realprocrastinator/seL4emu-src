/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

static inline int unknown_vcpu_exception_handler(vm_vcpu_t *vcpu, uint32_t hsr)
{
    return -1;
}

static inline int ignore_exception(vm_vcpu_t *vcpu, uint32_t hsr)
{
    return 0;
}
