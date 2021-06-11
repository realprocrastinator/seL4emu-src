/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

/***
 * @module guest_irq_controller.h
 * The libsel4vm IRQ controller interface provides a base abstraction around initialising a guest VM irq controller
 * and methods for injecting IRQs into a running VM instance.
 */

/**
 * Callback for irq acknowledgement
 */
typedef void (*irq_ack_fn_t)(vm_vcpu_t *vcpu, int irq, void *cookie);

/***
 * @function vm_inject_irq(vcpu, irq)
 * Inject an IRQ into a VM's interrupt controller
 * @param {vm_vcpu_t *} vcpu    Handle to the VCPU
 * @param {int} irq             IRQ number to inject
 * @return                      0 on success, otherwise -1 for error
 */
int vm_inject_irq(vm_vcpu_t *vcpu, int irq);

/***
 * @function vm_set_irq_level(vcpu, irq, irq_level)
 * Set level of IRQ number into a VM's interrupt controller
 * @param {vm_vcpu_t} vcpu      Handle to the VCPU
 * @param {int} irq             IRQ number to set level on
 * @param {int} irq_level       Value of IRQ level
 * @return                      0 on success, otherwise -1 for error
 */
int vm_set_irq_level(vm_vcpu_t *vcpu, int irq, int irq_level);

/***
 * @function vm_register_irq(vcpu, irq, ack_fn, cookie)
 * Register irq with an acknowledgment function
 * @param {vm_vcpu_t *} vcpu        Handle to the VCPU
 * @param {int} irq                 IRQ number to register acknowledgement function on
 * @param {irq_ack_fn_t} ack_fn     IRQ acknowledgement function
 * @param {void *} cookie           Cookie to pass back with IRQ acknowledgement function
 * @return                          0 on success, otherwise -1 for error
 */
int vm_register_irq(vm_vcpu_t *vcpu, int irq, irq_ack_fn_t ack_fn, void *cookie);

/***
 * @function vm_create_default_irq_controller(vm)
 * Install the default interrupt controller into the VM
 * @param {vm_t *} vm   Handle to the VM
 * @return              0 on success, otherwise -1 for error
 */
int vm_create_default_irq_controller(vm_t *vm);
