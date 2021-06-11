/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

/***
 * @module vpci.h
 * The libsel4vmmplatsupport vpci interface presents a Virtual PCI driver for ARM-based VM's.
 * Using the `vmm_pci_space_t` management interface, the vpci driver establishes the configuration
 * space in the guest VM. The driver also handles and processes all subsequent memory and ioport accesses to the
 * virtual pci device.
 */

#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/plat/vpci.h>

/* Mask to retrieve PCI bar size */
#define PCI_CFG_BAR_MASK 0xFFFFFFFF

/***
 * @function vm_install_vpci(vm, io_port, pci)
 * @param {vm_t *} vm                           A handle to the VM
 * @param {vmm_io_port_list_t *} io_port        IOPort library instance to emulate io accesses with
 * @param {vmm_pci_space_t } pci                PCI library instance to emulate PCI device accesses with
 * @return                                      0 for success, -1 for error
 */
int vm_install_vpci(vm_t *vm, vmm_io_port_list_t *io_port, vmm_pci_space_t *pci);

/***
 * @function fdt_generate_vpci_node(vm, pci, fdt, gic_phandle)
 * Generate a PCI device node for a given fdt. This taking into account
 * the virtual PCI device configuration space.
 * @param {vm_t *} vm               A handle to the VM
 * @param {vmm_pci_space_t *}       PCI library instance to generate fdt node
 * @param {void *} fdt              FDT blob to append generated device node
 * @param {int} gic_phandle         Phandle of IRQ controller to generate a correct interrupt map property
 * @return                          0 for success, -1 for error
 */
int fdt_generate_vpci_node(vm_t *vm, vmm_pci_space_t *pci, void *fdt, int gic_phandle);
