/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

enum img_type {
    /* Binary file, or unknown type */
    IMG_BIN,
    /* Elf file */
    IMG_ELF,
    /* u-boot image */
    IMG_UIMAGE,
    /* Android boot image */
    IMG_ABOOT,
    /* Self decompressing linux image file */
    IMG_ZIMAGE,
    /* Initrd image */
    IMG_INITRD_GZ,
    /* Flattened device tree blob */
    IMG_DTB,
};

struct guest_kernel_image_arch {};
