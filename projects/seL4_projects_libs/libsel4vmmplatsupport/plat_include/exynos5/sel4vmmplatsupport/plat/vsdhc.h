/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

const struct device dev_msh0;
const struct device dev_msh2;
int vm_install_nodma_sdhc0(vm_t *vm);
int vm_install_nodma_sdhc2(vm_t *vm);
