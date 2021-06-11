/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

typedef enum vm_exit_codes {
    VM_GUEST_MMIO_EXIT,
    VM_GUEST_IO_EXIT,
    VM_GUEST_NOTIFICATION_EXIT,
    VM_GUEST_UNKNOWN_EXIT
} vm_exit_codes_t;

#define VM_GUEST_ERROR_EXIT -1
