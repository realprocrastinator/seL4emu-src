/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <stdbool.h>

#include <sel4/sel4.h>
#include <sel4vm/guest_vm.h>

/***
 * @module guest_vcpu_fault.h
 * The libsel4vm VCPU fault interface provides a set of useful methods to query and configure vcpu objects that
 * have faulted during execution. This interface is commonly leveraged by VMM's to process a vcpu fault and handle
 * it accordingly.
 */

/***
 * @function get_vcpu_fault_address(vcpu)
 * Get current fault address of vcpu
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @return                      Current fault address of vcpu
 */
seL4_Word get_vcpu_fault_address(vm_vcpu_t *vcpu);

/***
 * @function get_vcpu_fault_ip(vcpu)
 * Get instruction pointer of current vcpu fault
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @return                      Intruction pointer of vcpu fault
 */
seL4_Word get_vcpu_fault_ip(vm_vcpu_t *vcpu);

/***
 * @function get_vcpu_fault_data(vcpu)
 * Get the data of the current vcpu fault
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @return                      Data of vcpu fault
 */
seL4_Word get_vcpu_fault_data(vm_vcpu_t *vcpu);

/***
 * @function get_vcpu_fault_data_mask(vcpu)
 * Get data mask of the current vcpu fault
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @return                      Data mask of vcpu fault
 */
seL4_Word get_vcpu_fault_data_mask(vm_vcpu_t *vcpu);

/***
 * @function get_vcpu_fault_size(vcpu)
 * Get access size of the current vcpu fault
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @return                      Access size of vcpu fault
 */
size_t get_vcpu_fault_size(vm_vcpu_t *vcpu);

/***
 * @function is_vcpu_read_fault(vcpu)
 * Is current vcpu fault a read fault
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @return                      True if read fault, False if write fault
 */
bool is_vcpu_read_fault(vm_vcpu_t *vcpu);

/***
 * @function set_vcpu_fault_data(vcpu, data)
 * Set the data of the current vcpu fault
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @param {seL4_Word} data      Data to set for current vcpu fault
 * @return 0 for success, otherwise -1 for error
 */
int set_vcpu_fault_data(vm_vcpu_t *vcpu, seL4_Word data);

/***
 * @function emulate_vcpu_fault(vcpu, data)
 * Emulate a read or write fault on a given data value
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 * @param {seL4_Word} data      Data to perform emulate fault on
 * @return                      Emulation result of vcpu fault over given data value
 */
seL4_Word emulate_vcpu_fault(vm_vcpu_t *vcpu, seL4_Word data);

/***
 * @function advance_vcpu_fault(vcpu)
 * Advance the current vcpu fault to the next stage/instruction
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 */
void advance_vcpu_fault(vm_vcpu_t *vcpu);

/***
 * @function restart_vcpu_fault(vcpu)
 * Restart the current vcpu fault
 * @param {vm_vcpu_t *} vcpu    Handle to vcpu
 */
void restart_vcpu_fault(vm_vcpu_t *vcpu);
