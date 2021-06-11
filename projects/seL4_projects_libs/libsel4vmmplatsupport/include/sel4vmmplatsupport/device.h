/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/***
 * @module device.h
 * The device.h interface provides a series of datastructures and helpers to manage VMM devices.
 */

#include <stdint.h>
#include <sel4vm/guest_vm.h>

/***
 * @struct device
 * Device Management Object
 * @param {const char *} name                                           A string representation of the device. Useful for debugging
 * @param {seL4_Word} pstart                                            The physical address of the device
 * @param {seL4_Word} size                                              Device mapping size
 * @param {int *(vm_t, vm_vcpu_t, dev, addr, len)} handle_device_fault  Fault handler
 * @param {void *} priv                                                 Device emulation private data
 *
 */
struct device {
    const char *name;
    seL4_Word pstart;
    seL4_Word size;
    int (*handle_device_fault)(vm_t *vm, vm_vcpu_t *vcpu, struct device *dev, uintptr_t addr, size_t len);
    void *priv;
};

/***
 * @struct device_list
 * Management for a list of devices
 * @param {struct device *} devices     List of registered devices
 * @param {int} num_devices             Total number of registered devices
 */
typedef struct device_list {
    struct device *devices;
    int num_devices;
} device_list_t;

/***
 * @function device_list_init(list)
 * Initialise an empty device list
 * @param {device_list_t *} list    device list to initialise
 * @return                          0 on success, otherwise -1 for error
 */
int device_list_init(device_list_t *list);

/***
 * @function add_device(dev_list, d)
 * Add a generic device to a given device list without performing any initialisation of the device
 * @param {device_list_t *} dev_list        A handle to the device list that the device should be installed into
 * @param {const struct device *} device    A description of the device
 * @return                                  0 on success, otherwise -1 for error
 */
int add_device(device_list_t *dev_list, const struct device *d);

/***
 * @function find_device_by_pa(dev_list, addr)
 * Find a device by a given addr within a device list
 * @param {device_list_t *} dev_list    Device list to search within
 * @param {uintptr_t} addr              Add to search with
 * @return                              Pointer to device if found, otherwise NULL if not found
 */
struct device *find_device_by_pa(device_list_t *dev_list, uintptr_t addr);
