/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/plat/device_map.h>
#include <sel4vmmplatsupport/plat/devices.h>

const struct device dev_i2c1 = {
    .name = "i2c1",
    .pstart = I2C1_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


const struct device dev_i2c2 = {
    .name = "i2c2",
    .pstart = I2C2_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


const struct device dev_i2c4 = {
    .name = "i2c4",
    .pstart = I2C4_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_i2chdmi = {
    .name = "i2c_hdmi",
    .pstart = I2C_HDMI_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


const struct device dev_usb2_ohci = {
    .name = "usb2.OHCI",
    .pstart = USB2_HOST_OHCI_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


const struct device dev_usb2_ehci = {
    .name = "usb2.EHCI",
    .pstart = USB2_HOST_EHCI_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


const struct device dev_usb2_ctrl = {
    .name = "usb2.ctrl",
    .pstart = USB2_HOST_CTRL_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_chip_id = {
    .name = "chipid",
    .pstart = CHIP_ID_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

/* Video */
const struct device dev_ps_tx_mixer = {
    .name = "tv_mixer",
    .pstart = TV_MIXER_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_hdmi0 = {
    .name = "hdmi0",
    .pstart = HDMI_0_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_hdmi1 = {
    .name = "hdmi1",
    .pstart = HDMI_1_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_hdmi2 = {
    .name = "hdmi2",
    .pstart = HDMI_2_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_hdmi3 = {
    .name = "hdmi3",
    .pstart = HDMI_3_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_hdmi4 = {
    .name = "hdmi4",
    .pstart = HDMI_4_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_hdmi5 = {
    .name = "hdmi5",
    .pstart = HDMI_5_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_hdmi6 = {
    .name = "hdmi6",
    .pstart = HDMI_6_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

/* DMA */
const struct device dev_ps_mdma0 = {
    .name = "MDMA0",
    .pstart = NS_MDMA0_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_mdma1 = {
    .name = "MDMA1",
    .pstart = NS_MDMA1_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_pdma0 = {
    .name = "PDMA0",
    .pstart = PDMA0_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_pdma1 = {
    .name = "PDMA1",
    .pstart = PDMA1_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


/* Timers */
const struct device dev_wdt_timer = {
    .name = "wdt",
    .pstart = WDT_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ps_pwm_timer = {
    .name = "pwm",
    .pstart = PWM_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


