/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

int vm_guest_write_mem(vm_t *vm, void *data, uintptr_t address, size_t size);

int vm_guest_read_mem(vm_t *vm, void *data, uintptr_t address, size_t size);
