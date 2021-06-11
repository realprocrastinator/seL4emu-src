/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_memory.h>

/***
 * @module guest_memory_helpers.h
 * The libsel4vm guest memory helpers interface provides simple utilities for using the guest memory interface.
 */

/***
 * @function default_error_fault_callback(vm, vcpu, fault_addr, fault_length, cookie)
 * Default fault callback that throws a fault error.
 * Useful to avoid having to re-define a fault callback on regions that should be mapped with all rights.
 * @param {vm_t *} vm               A handle to the VM
 * @param {vm_vcpu_t *} vcpu        A handle to the fault vcpu
 * @param {uintptr_t} fault addr    Faulting address
 * @param {size_t} fault_length     Length of faulted access
 * @param {void *} cookie           User cookie to pass onto callback
 * @return                          Always returns FAULT_ERROR
 */
memory_fault_result_t default_error_fault_callback(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr,
                                                   size_t fault_length, void *cookie);
