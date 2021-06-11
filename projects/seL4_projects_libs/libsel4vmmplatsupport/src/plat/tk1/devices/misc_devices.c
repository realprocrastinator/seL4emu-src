/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4vmmplatsupport/plat/device_map.h>
#include <sel4vmmplatsupport/device.h>

const struct device dev_usb1 = {
    .name = "usb3",
    .pstart = USB1_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};


const struct device dev_usb3 = {
    .name = "usb3",
    .pstart = USB3_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_sdmmc = {
    .name = "usb3",
    .pstart = SDMMC_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_rtc_kbc_pmc = {
    .name = "rtc_kbc_pmc",
    .pstart = RTC_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_data_memory = {
    .name = "data_memory",
    .pstart = DATA_MEMORY_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_exception_vectors = {
    .name = "exception_vectors",
    .pstart = EXCEPTION_VECTORS_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_system_registers = {
    .name = "system_registers",
    .pstart = SYSTEM_REGISTERS_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_ictlr = {
    .name = "ictlr",
    .pstart = ICTLR_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_apb_misc = {
    .name = "apb_misc",
    .pstart = APB_MISC_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_fuse = {
    .name = "fuse",
    .pstart = FUSE_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_gpios = {
    .name = "gpios",
    .pstart = GPIOS_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};
