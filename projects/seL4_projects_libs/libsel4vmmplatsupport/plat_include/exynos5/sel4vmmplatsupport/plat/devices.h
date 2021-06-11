/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

#include <sel4vmmplatsupport/arch/ac_device.h>
#include <sel4vmmplatsupport/device.h>

#include <sel4vmmplatsupport/plat/device_map.h>

int vm_install_vcmu_top(vm_t *vm);
int vm_install_vgpio_left(vm_t *vm);
int vm_install_vgpio_right(vm_t *vm);

extern const struct device dev_acp;
extern const struct device dev_i2c1;
extern const struct device dev_i2c2;
extern const struct device dev_i2c4;
extern const struct device dev_i2chdmi;
extern const struct device dev_usb2_ohci;
extern const struct device dev_usb2_ehci;
extern const struct device dev_usb2_ctrl;
extern const struct device dev_gpio_left;
extern const struct device dev_gpio_right;
extern const struct device dev_alive;
extern const struct device dev_ps_chip_id;
extern const struct device dev_cmu_top;
extern const struct device dev_cmu_core;
extern const struct device dev_cmu_cpu;
extern const struct device dev_cmu_mem;
extern const struct device dev_cmu_cdrex;
extern const struct device dev_cmu_isp;
extern const struct device dev_cmu_acp;
extern const struct device dev_sysreg;
extern const struct device dev_ps_msh0;
extern const struct device dev_ps_msh2;
extern const struct device dev_ps_tx_mixer;
extern const struct device dev_ps_hdmi0;
extern const struct device dev_ps_hdmi1;
extern const struct device dev_ps_hdmi2;
extern const struct device dev_ps_hdmi3;
extern const struct device dev_ps_hdmi4;
extern const struct device dev_ps_hdmi5;
extern const struct device dev_ps_hdmi6;
extern const struct device dev_ps_pdma0;
extern const struct device dev_ps_pdma1;
extern const struct device dev_ps_mdma0;
extern const struct device dev_ps_mdma1;
extern const struct device dev_ps_pwm_timer;
extern const struct device dev_ps_wdt_timer;


