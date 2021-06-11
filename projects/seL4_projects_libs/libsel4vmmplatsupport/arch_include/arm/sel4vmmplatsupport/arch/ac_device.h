/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/***
 * @module ac_device.h
 * The ARM access control device interface facilitates the creation of generic virtual devices in a VM instance with
 * access control permissions over the devices addressable memory. An access controlled device is often used to abstract
 * the memory of a specific platform hardware device e.g a clock device. This allows the user to present hardware devices
 * to a VM instance but limit their permissions with regards to modifying its register state.
 */

#include <sel4vm/guest_vm.h>

#include <sel4vmmplatsupport/device.h>

enum vacdev_default {
    VACDEV_DEFAULT_ALLOW,
    VACDEV_DEFAULT_DENY
};

enum vacdev_action {
    VACDEV_REPORT_ONLY,
    VACDEV_MASK_ONLY,
    VACDEV_REPORT_AND_MASK
};

/***
 * @function vm_install_generic_ac_device(vm, d, mask, size, action)
 * Installs a generic access controlled device
 * @param {vm_t *} vm                       The VM to install the device into
 * @param {const struct device *} d         A description of the device to install
 * @param {void *} mask                     An access mask. The mask provides a map of device bits that
 *                                          are modifiable by the guest.
 *                                          '1' represents bits that the guest can read and write
                                            '0' represents bits that can only be read by the guest
 *                                          Underlying memory for the mask should remain accessible for
 *                                          the life of this device. The mask may be updated at run time
 *                                          on demand.
 * @param {size_t} size                     The size of the mask. This is useful for conserving memory in
 *                                          cases where the underlying device does not occupy a full
 *                                          page. If an access lies outside of the range of the mask,
 *                                          guest access.
 * @param {enum vacdev_action} action       Action to take when access is violated.
 * @return                                  0 on success, -1 on error
 */
int vm_install_generic_ac_device(vm_t *vm, const struct device *d, void *mask,
                                 size_t size, enum vacdev_action action);
