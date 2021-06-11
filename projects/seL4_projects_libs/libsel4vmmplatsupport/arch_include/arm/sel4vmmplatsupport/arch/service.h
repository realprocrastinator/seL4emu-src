/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/**
 * Install a VMM service into the VM
 * Effectively a cap copy into the CNode of the VM. This allows the VM to
 * make a seL4 call
 * @param[in] vm       The VM to install the service into
 * @param[in] service  A capability for VM
 * @param[in] index    The index at which to install the cap
 * @param[in] badge    The badge to assign to the cap.
 * @return             0 on success
 */
int vmm_install_service(vm_t *vm, seL4_CPtr service, int index, uint32_t badge);
