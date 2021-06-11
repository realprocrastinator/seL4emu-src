/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

/***
 * @module ioports.h
 * The ioports interface provides a useful abstraction for initialising, registering and handling ioport events
 * for a guest VM instance. This is independent from the x86 libsel4vm ioports interface. This interface is intended
 * to be more generic and architecture independent, useful for usecases that require ioports without architecture
 * support (virtio PCI).
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef int (*ioport_in_fn)(void *cookie, unsigned int port_no, unsigned int size, unsigned int *result);
typedef int (*ioport_out_fn)(void *cookie, unsigned int port_no, unsigned int size, unsigned int value);

typedef enum ioport_type {
    IOPORT_FREE,
    IOPORT_ADDR
} ioport_type_t;

/***
 * @struct ioport_range
 * Range of ioport handler
 * @param {uint16_t} start  Start address of ioport range
 * @param {uint16_t} end    End address of ioport range
 * @param {uint16_t} size   Size of ioport range
 */
typedef struct ioport_range {
    uint16_t start;
    uint16_t end;
    uint16_t size;
} ioport_range_t;

/***
 * @struct ioport_interface
 * Datastructure used for ioport emulation, containing handlers for the ioport range
 * @param {void *} cookie           User supplied cookie to pass onto handlers
 * @param {ioport_in_fn} port_in    IO in operation handler
 * @param {ioport_out_fn} port_out  IO out operation handler
 * @param {const char *} desc       IOPort description, useful for debugging
 */
typedef struct ioport_interface {
    void *cookie;
    ioport_in_fn port_in;
    ioport_out_fn port_out;
    const char *desc;
} ioport_interface_t;

/***
 * @struct ioport_entry
 * Datastructure used to present a registered ioport range
 * @param {ioport_range_t} range            IO address range of ioport entry
 * @param {ioport_interface_t} interface    Emulation interface for ioport range
 */
typedef struct ioport_entry {
    ioport_range_t range;
    ioport_interface_t interface;
} ioport_entry_t;

/***
 * @struct vmm_io_list
 * Parent datastructure used to maintain a list of registered ioports
 * @param {int} num_ioports         Total number of registered ioports
 * @param {ioport_entry_t **}       List of registered ioport objects
 * @param {uint16_t} alloc_addr      Base ioport address we can safely bump allocate from, used when registering ioport handlers of type 'IOPORT_FREE'
 */
typedef struct vmm_io_list {
    int num_ioports;
    /* Sorted list of ioport functions */
    ioport_entry_t **ioports;
    uint16_t alloc_addr;
} vmm_io_port_list_t;

/***
 * @function vmm_io_port_init(io_list, ioport_alloc_addr)
 * Initialize the io port list manager.
 * @param {vmm_io_port_list_t **} io_list       Pointer to io_port list handle. This will be allocated and initialised
 * @param {uint16_t} ioport_alloc_addr          Base ioport address we can safely bump allocate from (doesn't conflict with other ioports).
 *                                              This is used when registering ioport handlers of type 'IOPORT_FREE'
 * @return                                      0 for success, otherwise -1 for error
 */
int vmm_io_port_init(vmm_io_port_list_t **io_list, uint16_t ioport_alloc_addr);

/***
 * @function vmm_io_port_add_handler(io_list, ioport_range, ioport_interface, port_type)
 * Add an io port range for emulation
 * @param {vmm_io_port_list_t *} io_list            Handle to ioport list. This is where the new ioport handler will be appended to
 * @param {ioport_range_t} ioport_range             Range the ioport handler will emulate
 * @param {ioport_interface_t} ioport_interface     IOPort emulation interface
 * @param {ioport_type_t} port_type                 The type of ioport being registered - IOPORT_FREE, IOPORT_ADDR
 * @return                                          NULL for error, otherwise pointer to newly created ioport entry
 */
ioport_entry_t *vmm_io_port_add_handler(vmm_io_port_list_t *io_list, ioport_range_t ioport_range,
                                        ioport_interface_t ioport_interface, ioport_type_t port_type);

/***
 * @function emulate_io_handler(io_port, port_no, is_in, size, data)
 * From a set of registered ioports, emulate an io instruction given a current ioport access.
 * @param {vmm_io_port_list_t *} io_port        List of registered ioports with in/out handlers
 * @param {unsigned int} port_no                IOPort address being accessed
 * @param {bool} is_in                          True if we are performing an io in operation, otherwise False
 * @param {size_t} size                         Size of io access
 * @param {unsigned int *} data                 Pointer with the data being written if io-out op, otherwise will be populated with data from an io-in op
 * @return                                      0 if handled, 1 if unhandled, otherwise -1 for error
 */
int emulate_io_handler(vmm_io_port_list_t *io_port, unsigned int port_no, bool is_in, size_t size, unsigned int *data);
