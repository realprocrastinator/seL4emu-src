/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

#define SYS_PA_TO_IPA 65
#define SYS_IPA_TO_PA 66
#define SYS_NOP 67

int vm_syscall_handler(vm_vcpu_t *vcpu);
