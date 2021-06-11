/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4/sel4.h>
#include <vka/vka.h>
#include <vka/capops.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/boot.h>

#include "vm.h"

int vm_run(vm_t *vm)
{
    return vm_run_arch(vm);
}

int vm_register_unhandled_mem_fault_callback(vm_t *vm, unhandled_mem_fault_callback_fn fault_handler,
                                             void *cookie)
{
    if (!vm) {
        ZF_LOGE("Failed to register mem fault callback: Invalid VM handle");
        return -1;
    }

    if (!fault_handler) {
        ZF_LOGE("Failed to register mem fault callback: Invalid handler");
        return -1;
    }
    vm->mem.unhandled_mem_fault_handler = fault_handler;
    vm->mem.unhandled_mem_fault_cookie = cookie;
    return 0;
}

int vm_register_notification_callback(vm_t *vm, notification_callback_fn notification_callback,
                                      void *cookie)
{
    if (!vm) {
        ZF_LOGE("Failed to register notification callback: Invalid VM handle");
        return -1;
    }

    if (!notification_callback) {
        ZF_LOGE("Failed to register notification callback: Invalid callback");
        return -1;
    }
    vm->run.notification_callback = notification_callback;
    vm->run.notification_callback_cookie = cookie;
    return 0;
}
