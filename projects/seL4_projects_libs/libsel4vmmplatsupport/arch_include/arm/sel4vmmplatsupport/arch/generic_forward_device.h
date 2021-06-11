/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

/***
 * @module generic_forward_device.h
 * This interface facilitates the creation of a virtual device used for
 * dispatching faults to external handlers. For example when using CAmkES,
 * a device frame for a real device can be given to a different CAmkES component
 * and this virtual device will forward read and write faults over a CAmkES
 * interface so the component can perform or emulate the actions.
 */

#include <stdint.h>
#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/device.h>

typedef void (*forward_write_fn)(uint32_t addr, uint32_t value);
typedef uint32_t (*forward_read_fn)(uint32_t addr);

/***
 * @struct generic_forward_cfg
 * Interface for forwarding read and write faults
 * @param {forward_write_fn} write_fn   A callback for forwarding write faults
 * @param {forward_read_fn} read_fn     A callback for forwarding read faults
 */
struct generic_forward_cfg {
    forward_write_fn write_fn;
    forward_read_fn read_fn;
};

/***
 * @function vm_install_generic_forward_device(vm, d, cfg)
 * Install the virtual forwarding device into a VM instance
 * @param {vm_t *} vm                   A handle to the VM
 * @param {const struct device *}       Virtual device being forwarded
 * @param {struct generic_forward_cfg}  Interface for forwarding the devices read and write faults
 * @return                              -1 for error, otherwise 0 for success
 */
int vm_install_generic_forward_device(vm_t *vm, const struct device *d,
                                      struct generic_forward_cfg cfg);
