/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/***
 * @module guest_vm_arch.h
 * The guest x86 vm interface is central to using libsel4vm on an x86 platform, providing definitions of the x86 guest vm
 * datastructures and primitives to configure the VM instance.
 */

#include <sel4/sel4.h>

#include <sel4vm/arch/vmexit_reasons.h>
#include <sel4vm/arch/ioports.h>

#define IO_APIC_DEFAULT_PHYS_BASE   0xfec00000
#define APIC_DEFAULT_PHYS_BASE      0xfee00000

typedef struct vm_lapic vm_lapic_t;
typedef struct i8259 i8259_t;
typedef struct guest_state guest_state_t;

/* Function prototype for vm exit handlers */
typedef int(*vmexit_handler_ptr)(vm_vcpu_t *vcpu);

/* A handler for a given vmcall function
 * Called in a vmcall exception if its token matches the vcpu register eax */
typedef int (*vmcall_handler)(vm_vcpu_t *vcpu);
typedef struct vmcall_handler {
    int token;
    vmcall_handler func;
} vmcall_handler_t;

/***
 * @struct vm_vcpu
 * Structure representing x86 specific vm properties
 * @param {vmexit_handler_ptr} vmexit_handler                           Set of exit handler hooks
 * @param {vmcall_handler_t *} vmcall_handlers                          Set of registered vmcall handlers
 * @param {unsigned int} vmcall_num_handler                             Total number of registered vmcall handlers
 * @param {uintptr_t} guest_pd                                          Guest physical address of where we built the vm's page directory
 * @param {unhandled_ioport_callback_fn} unhandled_ioport_callback      A callback for processing unhandled ioport faults
 * @param {void *} unhandled_ioport_callback_cookie                     A cookie to supply to the ioport callback
 * @param {vm_io_port_list_t} ioport_list                               List of registered ioport handlers
 * @param {i8259_t *} i8259_gs                                          PIC machine state
 */
struct vm_arch {
    vmexit_handler_ptr vmexit_handlers[VM_EXIT_REASON_NUM];
    vmcall_handler_t *vmcall_handlers;
    unsigned int vmcall_num_handlers;
    uintptr_t guest_pd;
    unhandled_ioport_callback_fn unhandled_ioport_callback;
    void *unhandled_ioport_callback_cookie;
    vm_io_port_list_t ioport_list;
    i8259_t *i8259_gs;
};

/***
 * @struct vm_vcpu_arch
 * Structure representing x86 specific vcpu properties
 * @param {guest_state_t *} guest_state         Current VCPU State
 * @param {vm_lapic_t *} lapic                  VM local apic
 */
struct vm_vcpu_arch {
    guest_state_t *guest_state;
    vm_lapic_t *lapic;
};
