/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

/***
 * @module vusb.h
 * The libsel4vmmplatsupport vusb interface presents a Virtual USB driver for ARM-based VM's.
 */

#include <autoconf.h>
#include <sel4vm/gen_config.h>
#include <usbdrivers/gen_config.h>
#ifdef CONFIG_LIB_USB

#include <usb/usb_host.h>
#include <sel4vm/guest_vm.h>
#include <sel4/sel4.h>

typedef struct vusb_device vusb_device_t;

/***
 * @function vm_install_vusb(vm, hcd, pbase, virq, vmm_ncap, vm_ncap, badge)
 * Install a virtual usb device
 * @param {vm_t *} vm               The VM in which to install the device
 * @param {usb_host_t *} hcd        The USB host controller that should be used for USB transactions.
 *                                  Calls made to this hcd may be redirected for filtering.
 * @param {uintptr_t} pbase         The guest physical address of the device (2 pages)
 * @param {int} virq                The virtual IRQ number for this device
 * @param {seL4_CPtr} vmm_ncap      The capability to the endpoint at which the VMM waits for notifications.
 * @param {seL4_CPtr} vm_ncap       The index at which to install a notification capability into the VM
 * @param {int} badge               The seL4 badge which should be applied to the notification capability.
 * @return                          A handle to the virtual usb device, or NULL on failure
 */
vusb_device_t *vm_install_vusb(vm_t *vm, usb_host_t *hcd, uintptr_t pbase,
                               int virq, seL4_CPtr vmm_ncap, seL4_CPtr vm_ncap,
                               int badge);


/***
 * @function vm_vusb_notify(vusb)
 * This function should be called when a notification is received from the
 * VM. The notification is identifyable by a message on the fault endpoint
 * of the VM which has a badge that matches that which was passed into the
 * vm_install_vusb function.
 * @param {vusb_device_t *} vusb        A handle to a virtual usb device
 */
void vm_vusb_notify(vusb_device_t *vusb);

#endif /* CONFIG_LIB_USB */
