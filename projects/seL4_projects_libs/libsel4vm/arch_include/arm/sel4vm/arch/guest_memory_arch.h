/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

typedef struct vm vm_t;

/**
 * Convert an intermediate physical address belonging to a guest VM to a physical address
 * @param[in] vm            A handle to the vm
 * @param[in] ipa_base      The intermediate physical address
 * @param[in] size          Size of the memory region
 * @return                  Physical address value
 */
uintptr_t vm_arm_ipa_to_pa(vm_t *vm, uintptr_t ipa_base, size_t size);
