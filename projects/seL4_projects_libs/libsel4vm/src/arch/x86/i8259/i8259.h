/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <sel4vm/guest_vm.h>

/* Init function */
int i8259_pre_init(vm_t *vm);

/* Functions to retrieve interrupt state */
int i8259_get_interrupt(vm_t *vm);
int i8259_has_interrupt(vm_t *vm);
