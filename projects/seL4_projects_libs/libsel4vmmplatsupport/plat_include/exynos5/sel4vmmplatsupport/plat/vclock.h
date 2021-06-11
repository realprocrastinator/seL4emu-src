/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

#include <sel4vmmplatsupport/arch/ac_device.h>
#include <sel4vmmplatsupport/device.h>

/**
 * Clock Access control device
 */
struct clock_device;

/**
 * Install a CLOCK access control
 * @param[in] vm         The VM to install the device into
 * @param[in] default_ac The default access control state to apply
 * @param[in] action     Action to take when access is violated
 * @return               A handle to the GPIO Access control device, NULL on
 *                       failure
 */
struct clock_device *vm_install_ac_clock(vm_t *vm, enum vacdev_default default_ac,
                                         enum vacdev_action action);

/**
 * Provide clock access to the VM
 * @param[in] clock_device  A handle to the clock Access Control device
 * @param[in] clk_id        The ID of the clock to provide access to
 * @return                  0 on success
 */
int vm_clock_provide(struct clock_device *clock_device, enum clk_id clk_id);

/**
 * Deny clock access to the VM
 * @param[in] clock_device  A handle to the clock Access Control device
 * @param[in] clk_id        The ID of the clock to deny access to
 * @return                  0 on success
 */
int vm_clock_restrict(struct clock_device *clock_device, enum clk_id clk_id);
