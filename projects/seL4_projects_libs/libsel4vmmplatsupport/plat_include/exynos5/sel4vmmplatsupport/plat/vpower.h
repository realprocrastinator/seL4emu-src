/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

typedef int (*vm_power_cb)(vm_t *vm, void *token);
int vm_install_vpower(vm_t *vm, vm_power_cb shutdown_cb, void *shutdown_token,
                      vm_power_cb reboot_cb, void *reboot_token);
