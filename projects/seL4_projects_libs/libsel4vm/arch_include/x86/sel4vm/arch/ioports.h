/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

/***
 * @module ioports.h
 * The x86 ioports interface provides a useful abstraction for initialising, registering and handling ioport events
 * for a guest VM instance. IOPort faults are directed through this interface.
 */

#include <stdint.h>

#include <sel4/sel4.h>
#include <simple/simple.h>

typedef struct vm vm_t;
typedef struct vm_vcpu vm_vcpu_t;

typedef enum ioport_fault_result {
    IO_FAULT_HANDLED,
    IO_FAULT_UNHANDLED,
    IO_FAULT_ERROR
} ioport_fault_result_t;

/**
 * Type signature of ioport in handler function.
 * @param {vm_vcpu_t *} vcpu        A handle to the VCPU handling the ioport in operation
 * @param {void *} cookie           A cookie to supply to the handler
 * @param {unsigned int} port_no    Base port address being accessed
 * @param {unsigned int} size       Size of ioport access
 * @param {unsigned int} result     Pointer referencing the resulting value of the io operation. The handler is expected to populate
 *                                  the variable with the value being read
 * @return                          IOPort fault handling status code: IO_FAULT_HANDLED, IO_FAULT_UNHANDLED, IO_FAULT_ERROR
 */
typedef ioport_fault_result_t (*vm_ioport_in_fn)(vm_vcpu_t *vcpu, void *cookie, unsigned int port_no, unsigned int size,
                                                 unsigned int *result);

/**
 * Type signature of ioport out handler function.
 * @param {vm_vcpu_t *} vcpu        A handle to the VCPU handling the ioport in operation
 * @param {void *} cookie           A cookie to supply to the handler
 * @param {unsigned int} port_no    Base port address being accessed
 * @param {unsigned int} size       Size of ioport access
 * @param {unsigned int} value      Value being written in ioport out operation
 * @return                          IOPort fault handling status code: IO_FAULT_HANDLED, IO_FAULT_UNHANDLED, IO_FAULT_ERROR
 */
typedef ioport_fault_result_t (*vm_ioport_out_fn)(vm_vcpu_t *vcpu, void *cookie, unsigned int port_no,
                                                  unsigned int size,
                                                  unsigned int value);

/**
 * Type signature of unhandled ioport fault function, invoked when a ioport fault is unable to be handled
 * @param {vm_vcpu_t *} vcpu            A handle to the VCPU object invoking unhandled ioport operation
 * @param {unsigned int} port_no        Base port address being accessed
 * @param {bool} is_in                  True if it is an ioport in access. False if it is an ioport out access.
 * @param {unsigned int *} result       Pointer referencing the resulting value of the io operation. If it is an ioport in operation,
 *                                      the handler is expected to populate the variable with the value being read. Otherwise if it
 *                                      is an ioport out operation, then the derefenced value is the value being written.
 * @param {size_t} size                 Size of ioport fault
 * @param {void *} cookie               User cookie to pass onto callback
 * @return                              Fault handling status code: HANDLED, UNHANDLED, RESTART, ERROR
 */
typedef ioport_fault_result_t (*unhandled_ioport_callback_fn)(vm_vcpu_t *vcpu, unsigned int port_no, bool is_in,
                                                              unsigned int *value,
                                                              size_t size, void *cookie);

typedef struct vm_ioport_range {
    uint16_t start;
    uint16_t end;
} vm_ioport_range_t;

typedef struct vm_ioport_interface {
    void *cookie;
    /* ioport handler functions */
    vm_ioport_in_fn port_in;
    vm_ioport_out_fn port_out;
    /* ioport description (for debugging) */
    const char *desc;
} vm_ioport_interface_t;

typedef struct vm_ioport_entry {
    vm_ioport_range_t range;
    vm_ioport_interface_t interface;
} vm_ioport_entry_t;

typedef struct vm_io_list {
    int num_ioports;
    /* Sorted list of ioport functions */
    vm_ioport_entry_t *ioports;
} vm_io_port_list_t;

/***
 * @function vm_io_port_add_handler(vm, ioport_range, ioport_interface)
 * Add an io port range for emulation
 * @param {vm_t *} vm                               A handle to the VM
 * @param {vm_ioport_range_t} ioport_range          Range of ioport being emulated with the given handler
 * @param {vm_ioport_interface_t} ioport_interface  Interface for ioport range, containing io_in and io_out handler functions
 * @return                                          0 for success, -1 for error
 */
int vm_io_port_add_handler(vm_t *vm, vm_ioport_range_t ioport_range,
                           vm_ioport_interface_t ioport_interface);

/***
 * @function vm_register_unhandled_ioport_callback(vm, ioport_callback, cookie)
 * Register a callback for processing unhandled ioport faults (faults unknown to libsel4vm)
 * @param {vm_t *} vm                                       A handle to the VM
 * @param {unhandled_ioport_callback_fn} ioport_callback    A user supplied callback to process unhandled ioport faults
 * @param {void *} cookie                                   A cookie to supply to the callback
 * @return                                                  0 for success, -1 for error
 */
int vm_register_unhandled_ioport_callback(vm_t *vm, unhandled_ioport_callback_fn ioport_callback,
                                          void *cookie);

/***
 * @function vm_enable_passthrough_ioport(vcpu, port_start, port_end)
 * Enable the passing-through of specific ioport ranges to the VM
 * @param {vm_vcpu_t *} vcpu            A handle to the VCPU being given ioport passthrough access
 * @param {uint16_t} port_start         Base address of ioport
 * @param {uint16_t} port_end           End address of ioport
 * @return                              0 for success, -1 for error
 */
int vm_enable_passthrough_ioport(vm_vcpu_t *vcpu, uint16_t port_start, uint16_t port_end);
