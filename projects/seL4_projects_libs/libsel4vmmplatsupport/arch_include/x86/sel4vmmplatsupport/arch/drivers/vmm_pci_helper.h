/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/***
 * @module vmm_pci_helper.h
 * The interface presents a series of helpers for establishing VMM PCI support on x86 platforms.
 */

#include <sel4vm/guest_vm.h>
#include <sel4vm/arch/ioports.h>
#include <sel4vmmplatsupport/drivers/pci_helper.h>

#include <pci/virtual_pci.h>
#include <pci/helper.h>

/***
 * @function vmm_pci_helper_map_bars(vm, cfg, bars)
 * Given a PCI device config, map the PCI device bars into the VM, effectively passing-through the
 * PCI device. This will map MMIO and IO-based bars.
 * @param {vm_t *} vm                       A handle to the VM
 * @param {libpci_device_iocfg_t *} cfg     PCI device config
 * @param {vmm_pci_bar_t *} bars            Resulting PCI bars mapped into the VM
 * @return                                  -1 for error, otherwise the number of bars mapped into the VM (>=0)
 */
int vmm_pci_helper_map_bars(vm_t *vm, libpci_device_iocfg_t *cfg, vmm_pci_bar_t *bars);

/* Functions for emulating PCI config spaces over IO ports */
/***
 * @function vmm_pci_io_port_in(vcpu, cookie, port_no, size, result)
 * Emulates IOPort in access on the VMM Virtual PCI device
 * @param {vm_vcpu_t *} vcpu            Faulting vcpu performing ioport access
 * @param {unsigned int} port_no        Port address being accessed
 * @param {unsigned int} size           Size of ioport access
 * @param {unsigned int *} result       Pointer that will be populated with resulting data of io-in op
 */
ioport_fault_result_t vmm_pci_io_port_in(vm_vcpu_t *vcpu, void *cookie, unsigned int port_no, unsigned int size,
                                         unsigned int *result);

/***
 * @function vmm_pci_io_port_out(vcpu, cookie, port_no, size, value)
 * Emulates IOPort out access on the VMM Virtual PCI device
 * @param {vm_vcpu_t *} vcpu            Faulting vcpu performing ioport access
 * @param {unsigned int} port_no        Port address being accessed
 * @param {unsigned int} size           Size of ioport access
 * @param {unsigned int} value          Value being written in io-out op
 */
ioport_fault_result_t vmm_pci_io_port_out(vm_vcpu_t *vcpu, void *cookie, unsigned int port_no, unsigned int size,
                                          unsigned int value);
