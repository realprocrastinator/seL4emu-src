/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

/***
 * @module vmcall.h
 * The x86 vmcall interface provides methods for registering and managing vmcall handlers. These being used
 * to process x86 guest hypercalls though the vmcall instruction.
 */

#include <sel4vm/guest_vm.h>

/***
 * @function vm_reg_new_vmcall_handler(vm, func, token)
 * Register a new vmcall handler. The being hypercalls invoked by the
 * guest through the vmcall instruction.
 * @param {vm_t *} vm               A handle to the VM
 * @param {vmcall_handler} func     A handler function for the given vmcall being registered
 * @param {int} token               A token to associate with a vmcall handler.
 *                                  This being matched with the value found in the vcpu EAX register on a vmcall exception
 * @return                          0 on success, -1 on error
 */
int vm_reg_new_vmcall_handler(vm_t *vm, vmcall_handler func, int token);
