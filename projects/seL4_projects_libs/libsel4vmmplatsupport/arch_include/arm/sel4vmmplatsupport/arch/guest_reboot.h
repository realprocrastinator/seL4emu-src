/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/***
 * @module guest_reboot.h
 * The guest reboot interface provides a series of helpers for registering callbacks when rebooting the VMM.
 * This interface giving various VMM components and drivers the ability to reset necessary state on a reboot.
 */

typedef int (*reboot_hook_fn)(vm_t *vm, void *token);

/***
 * @struct reboot_hook
 * Datastructure representing a reboot hook, containing a callback function to invoke when processing the
 * hook
 * @param {reboot_hook_fn} fn       Function pointer to reboot callback
 * @param {void *} token            Cookie passed to reboot callback when invoked
 */
typedef struct reboot_hook {
    reboot_hook_fn fn;
    void *token;
} reboot_hook_t;

/***
 * @struct reboot_hooks_list
 * Reboot hooks management datastructure. Contains a list of reboot hooks that a VMM registers
 * @param {reboot_hook_t *} rb_hooks        List of reboot hooks
 * @param {size_t} nhooks                   Number of reboot hooks in `rb_hooks` member
 */
typedef struct reboot_hooks_list {
    reboot_hook_t *rb_hooks;
    size_t nhooks;
} reboot_hooks_list_t;

/***
 * @function vmm_init_reboot_hooks_list(rb_hooks_list)
 * Initialise state of a given reboot hooks list
 * @param {reboot_hooks_list_t *} rb_hooks_list     Handle to reboot hooks list
 * @return                                          0 for success, otherwise -1 for error
 */
int vmm_init_reboot_hooks_list(reboot_hooks_list_t *rb_hooks_list);

/***
 * @function vmm_register_reboot_callback(rb_hooks_list, hook, token)
 * Register a reboot callback within a given reboot hooks list
 * @param {reboot_hooks_list_t *} rb_hooks_list     Handle to reboot hooks list
 * @param {rb_hook_fn} hook                         Reboot callback to be invoked when list is processed
 * @param {void *} token                            Cookie passed to reboot callback when invoked
 * @return                                          0 for success, otherwise -1 for error
 */
int vmm_register_reboot_callback(reboot_hooks_list_t *rb_hooks_list, reboot_hook_fn hook, void *token);

/***
 * @function vmm_process_reboot_callbacks(vm, rb_hooks_list)
 * Process the reboot hooks registered in a reboot hooks list
 * @param {vm_t *} vm                               Handle to vm - passed onto reboot callback
 * @param {reboot_hooks_list_t *} rb_hooks_list     Handle to reboot hooks list
 * @return                                          0 for success, otherwise -1 for error
 */
int vmm_process_reboot_callbacks(vm_t *vm, reboot_hooks_list_t *rb_hooks_list);
