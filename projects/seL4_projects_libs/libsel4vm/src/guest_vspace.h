/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4/sel4.h>
#include <vspace/vspace.h>
#include <vka/vka.h>

/* Constructs a vspace that will duplicate mappings between a page directory and several IO spaces */
int vm_init_guest_vspace(vspace_t *loader, vspace_t *vmm, vspace_t *new_vspace, vka_t *vka, seL4_CPtr page_directory);
