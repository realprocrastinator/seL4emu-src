/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4/sel4.h>
#include <sel4vm/guest_vm.h>

/***
 * @module guest_arm_context.h
 * The libsel4vm ARM context interface provides a set of useful getters and setters on ARM vcpu thread contexts.
 * This interface is commonly leveraged by VMM's to initialise a vcpu state, process a vcpu fault and
 * accordingly update its state.
 */

/***
 * @function vm_set_thread_context(vcpu, context)
 * Set a VCPU's thread registers given a TCB user context
 * @param {vm_vcpu_t *} vcpu            Handle to the vcpu
 * @param {seL4_UserContext} context    seL4_UserContext applied to VCPU's TCB
 * @return                              0 on success, otherwise -1 for error
 */
int vm_set_thread_context(vm_vcpu_t *vcpu, seL4_UserContext context);

/***
 * @function vm_set_thread_context_reg(vcpu, reg, value)
 * Set a single VCPU's TCB register
 * @param {vm_vcpu_t *} vcpu        Handle to the vcpu
 * @param {unsigned int} reg        Index offset of register in seL4_UserContext e.g pc (seL4_UserContext.pc) => 0
 * @param {uintptr_t} value         Value to set TCB register with
 * @return                          0 on success, otherwise -1 for error
 */
int vm_set_thread_context_reg(vm_vcpu_t *vcpu, unsigned int reg, uintptr_t value);

/***
 * @function vm_get_thread_context(vcpu, context)
 * Get a VCPU's TCB user context
 * @param {vm_vcpu_t} vcpu              Handle to the vcpu
 * @param {seL4_UserContext} context    Pointer to user supplied seL4_UserContext to populate with VCPU's TCB user context
 * @return                              0 on success, otherwise -1 for error
 */
int vm_get_thread_context(vm_vcpu_t *vcpu, seL4_UserContext *context);

/***
 * @function vm_get_thread_context_reg(vcpu, reg, value)
 * Get a single VCPU's TCB register
 * @param {vm_vcpu_t *} vcpu        Handle to the vcpu
 * @param {unsigned int} reg        Index offset of register in seL4_UserContext e.g pc (seL4_UserContext.pc) => 0
 * @param {uintptr_t *} value       Pointer to user supplied variable to populate TCB register value with
 * @return                          0 on success, otherwise -1 for error
 */
int vm_get_thread_context_reg(vm_vcpu_t *vcpu, unsigned int reg, uintptr_t *value);

/* ARM VCPU Register Getters and Setters */

/***
 * @function vm_set_arm_vcpu_reg(vcpu, reg, value)
 * Set an ARM VCPU register
 * @param {vm_vcpu_t *} vcpu        Handle to the vcpu
 * @param {seL4_Word} reg           VCPU Register field defined in seL4_VCPUReg
 * @param {uintptr_t *} value       Value to set VCPU register with
 * @return                          0 on success, otherwise -1 for error
 */
int vm_set_arm_vcpu_reg(vm_vcpu_t *vcpu, seL4_Word reg, uintptr_t value);

/***
 * @function vm_get_arm_vcpu_reg(vcpu, reg, value)
 * Get an ARM VCPU register
 * @param {vm_vcpu_t *} vcpu    Handle to the vcpu
 * @param {seL4_Word} reg       VCPU Register field defined in seL4_VCPUReg
 * @param {uintptr_t *} value   Pointer to user supplied variable to populate VCPU register value with
 * @return                      0 on success, otherwise -1 for error
 */
int vm_get_arm_vcpu_reg(vm_vcpu_t *vcpu, seL4_Word reg, uintptr_t *value);
