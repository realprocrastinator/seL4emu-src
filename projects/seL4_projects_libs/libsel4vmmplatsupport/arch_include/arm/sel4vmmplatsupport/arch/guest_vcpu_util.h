/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/***
 * @module guest_vcpu_util.h
 * The ARM guest vcpu util interface provides abstractions and helpers for managing libsel4vm vcpus on an ARM platform.
 */

#include <sel4vmmplatsupport/plat/guest_vcpu_util.h>

/***
 * @function fdt_generate_plat_vcpu_node(vm, fdt)
 * Generate a CPU device node for a given fdt. This taking into account
 * the vcpus created for the VM.
 * @param {vm_t *} vm       A handle to the VM
 * @param {void *} fdt      FDT blob to append generated device node
 * @return                  0 for success, -1 for error
 */
int fdt_generate_plat_vcpu_node(vm_t *vm, void *fdt);
