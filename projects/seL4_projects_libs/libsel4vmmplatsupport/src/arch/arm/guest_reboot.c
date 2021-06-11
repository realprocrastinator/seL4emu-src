/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include <sel4utils/util.h>
#include <sel4vm/guest_vm.h>

#include <sel4vmmplatsupport/arch/guest_reboot.h>

int vmm_register_reboot_callback(reboot_hooks_list_t *rb_hooks_list, reboot_hook_fn hook, void *token)
{
    if (rb_hooks_list == NULL) {
        ZF_LOGE("rb hooks list is NULL");
        return -1;
    }
    if (hook == NULL) {
        ZF_LOGE("hook is NULL");
        return -1;
    }
    reboot_hook_t rb = {
        .fn = hook,
        .token = token
    };

    reboot_hook_t *new_hooks = realloc(rb_hooks_list->rb_hooks, sizeof(reboot_hook_t) * rb_hooks_list->nhooks + 1);
    if (!new_hooks) {
        ZF_LOGE("Failed to allocate memory for new reboot hook");
        return -1;
    }
    rb_hooks_list->rb_hooks = new_hooks;
    rb_hooks_list->rb_hooks[rb_hooks_list->nhooks++] = rb;
    return 0;
}

int vmm_process_reboot_callbacks(vm_t *vm, reboot_hooks_list_t *rb_hooks_list)
{
    for (int i = 0; i < rb_hooks_list->nhooks; i++) {
        reboot_hook_t rb = rb_hooks_list->rb_hooks[i];
        if (rb.fn == NULL) {
            ZF_LOGE("NULL call back has been registered");
            return -1;
        }
        int err = rb.fn(vm, rb.token);
        if (err) {
            ZF_LOGE("Callback returned error: %d", err);
            return -1;
        }
    }
    return 0;
}

int vmm_init_reboot_hooks_list(reboot_hooks_list_t *rb_hooks_list)
{
    rb_hooks_list->nhooks = 0;
    rb_hooks_list->rb_hooks = NULL;
    return 0;
}
