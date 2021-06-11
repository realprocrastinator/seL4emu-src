/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

/***
 * @module guest_vcpu_fault.h
 * The arm guest vcpu fault interface provides a module for registering
 * and processing vcpu faults. The module can be used such that it directly hooks
 * into an ARM VMMs unhandled vcpu callback interface or wrapped over by another driver. This
 * module particularly provides useful handlers to vcpu faults relating to SMC and MSR/MRS instructions.
 */

typedef int (*vcpu_exception_handler_fn)(vm_vcpu_t *vcpu, uint32_t hsr);

/***
 * @function vmm_handle_arm_vcpu_exception(vcpu, hsr, cookie)
 * Handle a vcpu exception given the HSR value - Syndrome information
 * @param {vm_vcpu_t *} vcpu        A handle to the faulting VCPU
 * @param {uint32_t } hsr           Syndrome information value describing the exception/fault
 * @param {void *} cookie           User supplied cookie to pass onto exception
 * @return                          -1 on error, otherwise 0 for success
 */
int vmm_handle_arm_vcpu_exception(vm_vcpu_t *vcpu, uint32_t hsr, void *cookie);

/***
 * @function register_arm_vcpu_exception_handler(ec_class, exception_handler)
 * Register a handler to a vcpu exception class
 * @param {uint32_t} ec_class                               The exception class the handler will be called on
 * @param {vcpu_exception_handler_fn} exception_handler     Function pointer to the exception handler
 */
int register_arm_vcpu_exception_handler(uint32_t ec_class, vcpu_exception_handler_fn exception_handler);
