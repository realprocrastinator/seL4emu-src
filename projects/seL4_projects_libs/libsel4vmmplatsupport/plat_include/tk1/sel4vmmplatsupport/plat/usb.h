/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

#include <sel4vmmplatsupport/arch/guest_reboot.h>

int vm_install_tk1_usb_passthrough_device(vm_t *vm, reboot_hooks_list_t *reboot_hooks);
