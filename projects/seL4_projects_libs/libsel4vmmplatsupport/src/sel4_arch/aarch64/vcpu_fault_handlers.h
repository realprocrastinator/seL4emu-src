/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>
#include <sel4vm/arch/processor.h>
#include <sel4vm/sel4_arch/processor.h>
#include <sel4vmmplatsupport/arch/guest_vcpu_fault.h>

#include "vcpu_fault.h"
#include "sysreg_exception.h"
#include "smc.h"

int software_breakpoint_exception(vm_vcpu_t *vcpu, uint32_t hsr);

static vcpu_exception_handler_fn vcpu_exception_handlers[] = {
    [0 ... HSR_MAX_EXCEPTION] = unknown_vcpu_exception_handler,
    [HSR_WFx_EXCEPTION] = ignore_exception,
    [HSR_SYSREG_64_EXCEPTION] = sysreg_exception_handler,
    [HSR_SWBRK_64_EXCEPTION] = software_breakpoint_exception,
    [HSR_SMC_64_EXCEPTION] = handle_smc
};
