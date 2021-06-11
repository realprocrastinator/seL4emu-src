/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

/**
 * vm_combiner_irq_handler should be called when an IRQ combiner IRQ occurs.
 * The caller is responsible for acknowledging the IRQ once this function
 * returns
 */
int vm_install_vcombiner(vm_t *vm);
void vm_combiner_irq_handler(vm_t *vm, int irq);
