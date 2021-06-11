/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <autoconf.h>
#include <sel4vm/gen_config.h>
#include <sel4vm/guest_vm.h>

#ifndef COLOUR
#define COLOUR "\033[;1;%dm"
#define COLOUR_R "\033[;1;31m"
#define COLOUR_G "\033[;1;32m"
#define COLOUR_Y "\033[;1;33m"
#define COLOUR_B "\033[;1;34m"
#define COLOUR_M "\033[;1;35m"
#define COLOUR_C "\033[;1;36m"
#define COLOUR_RESET "\033[0m"
#endif

void vm_print_guest_context(vm_vcpu_t *);

