/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/***
 * @module pci.h
 * This interface presents a VMM PCI Driver. This manages the host's PCI devices, and handles guest OS PCI config space
 * read & writes.
 */

#include <stdint.h>

/***
 * @struct vmm_pci_address
 * Represents a PCI address by Bus/Device/Function
 * @param {uint8_t} bus     Bus value
 * @param {uint8_t} dev     Device value
 * @param {uint8_t} fun     Function value
 */
typedef struct vmm_pci_address {
    uint8_t bus;
    uint8_t dev;
    uint8_t fun;
} vmm_pci_address_t;

/* Functions for accessing the pci config space */
typedef struct vmm_pci_config {
    void *cookie;
    uint8_t (*ioread8)(void *cookie, vmm_pci_address_t addr, unsigned int offset);
    uint16_t (*ioread16)(void *cookie, vmm_pci_address_t addr, unsigned int offset);
    uint32_t (*ioread32)(void *cookie, vmm_pci_address_t addr, unsigned int offset);
    void (*iowrite8)(void *cookie, vmm_pci_address_t addr, unsigned int offset, uint8_t val);
    void (*iowrite16)(void *cookie, vmm_pci_address_t addr, unsigned int offset, uint16_t val);
    void (*iowrite32)(void *cookie, vmm_pci_address_t addr, unsigned int offset, uint32_t val);
} vmm_pci_config_t;

/***
 * @struct vmm_pci_entry
 * Abstracts the virtual PCI device. This is is inserted into the virtual PCI configuration space
 * @param {void *} cookie                                                           User supplied cookie to pass onto callback functions
 * @param {int *(void *cookie, int offset, int size, uint32_t *result)} ioread      Configuration space read callback
 * @param {int *(void *cookie, int offset, int size, uint32_t value)} iowrite       Configuration space write callback
 */
typedef struct vmm_pci_entry {
    void *cookie;
    int (*ioread)(void *cookie, int offset, int size, uint32_t *result);
    int (*iowrite)(void *cookie, int offset, int size, uint32_t value);
} vmm_pci_entry_t;

/***
 * @struct vmm_pci_sapce
 * Represents a single host virtual PCI space
 * @param {vmm_pci_entry_t *} bus       The PCI bus, representing 32 devices, each of which has 8 functions
                                        This only supports one bus at the moment.
 * @param {uint32_t} conf_port_addr     The current config address for IO port emulation
 */
typedef struct vmm_pci_space {
    vmm_pci_entry_t *bus0[32][8];
    uint32_t conf_port_addr;
} vmm_pci_space_t;

/***
 * @function vmm_pci_init(space)
 * Initialize PCI space
 * @param {vmm_pci_space_t **} space    Pointer to PCI space being initialised
 * @return                              0 on success, -1 on error
 */
int vmm_pci_init(vmm_pci_space_t **space);

/***
 * @function vmm_pci_add_entry(space, entry, addr)
 * Add a PCI entry. Optionally reports where it is located
 * @param {vmm_pci_space_t *} space         PCI space handle
 * @param {vmm_pci_entry_t} entry           PCI entry being addr
 * @param {vmm_pci_addr_t *} addr           Resulting PCI address where entry gets located
 * @return                                  0 on success, -1 on error
 */
int vmm_pci_add_entry(vmm_pci_space_t *space, vmm_pci_entry_t entry, vmm_pci_address_t *addr);

/***
 * @function make_addr_reg_from_config(conf, addr, reg)
 * Convert config to pci address
 * @param {uint32_t} conf                   Configuration value to convert to pci address
 * @param {vmm_pci_address_t *} addr        Resulting PCI address
 * @param {uint8_t *} reg                   Resulting register value
 */
void make_addr_reg_from_config(uint32_t conf, vmm_pci_address_t *addr, uint8_t *reg);

/***
 * @function find_device(self, addr)
 * Find PCI device given a PCI address (Bus/Dev/Func)
 * @param {vmm_pci_space_t *} self      PCI space handle
 * @param {vmm_pci_address_t} addr      PCI address of device
 * @return                              NULL on error, otherwise pointer to registered pci entry
 */
vmm_pci_entry_t *find_device(vmm_pci_space_t *self, vmm_pci_address_t addr);
