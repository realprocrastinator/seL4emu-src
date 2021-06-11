/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

int vm_init_arch(vm_t *vm);
int vm_create_vcpu_arch(vm_t *vm, vm_vcpu_t *vcpu);
