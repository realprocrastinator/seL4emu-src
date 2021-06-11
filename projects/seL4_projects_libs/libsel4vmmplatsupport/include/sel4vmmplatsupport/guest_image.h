/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/***
 * @module guest_image.h
 * The guest image interface provides general utilites to load guest vm images (e.g. kernel, initrd, modules).
 * This interface probably stops being relevant/useful after we start running a VM instance.
 */

#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/arch/guest_image_arch.h>

typedef struct guest_kernel_image_arch guest_kernel_image_arch_t;

/***
 * @struct guest_image
 * General datastructure for managing a guest image
 * @param {uintptr_t} load_paddr    Base address (in guest physical) where the image was loaded
 * @param {size_t} alignment        Alignment we used when loading the image
 * @param {size_t} size             Size of guest image
 */
typedef struct guest_image {
    uintptr_t load_paddr;
    size_t alignment;
    size_t size;
} guest_image_t;

/***
 * @struct guest_kernel_image
 * Stores information about the guest kernel image we are loading.
 * @param {guest_image_t} kernel_image                      Datastructure referring to guest kernel image
 * @param {guest_kernel_image_arch_t} kernel_image_arch     Architecture specific information for loaded guest image
 */
typedef struct guest_kernel_image {
    guest_image_t kernel_image;
    guest_kernel_image_arch_t kernel_image_arch;
} guest_kernel_image_t;

/***
 * @function vm_load_guest_kernel(vm, kernel_name, load_address, alignment, guest_kernel_image)
 * Load guest kernel image
 * @param {vm_t *} vm                                           Handle to the VM
 * @param {const char *} kernel_name                            Name of the kernel image
 * @param {uintptr_t} load_address                              Address to load guest kernel image at
 * @param {size_t} alignment                                    Alignment for loading kernel image
 * @param {guest_kernel_image_t *} guest_kernel_image           Handle to information regarding the resulted loading of the guest kernel image
 * @return                                                      0 on success, otherwise -1 on error
 */
int vm_load_guest_kernel(vm_t *vm, const char *kernel_name, uintptr_t load_address, size_t alignment,
                         guest_kernel_image_t *guest_kernel_image);

/***
 * @function vm_load_guest_module(vm, module_name, load_address, alignment, guest_image)
 * Load guest kernel module e.g. initrd
 * @param {vm_t *} vm                           Handle to the VM
 * @param {const char *} module_name            Name of the module image
 * @param {uintptr_t} load_address              Address to load guest kernel image at
 * @param {size_t} alignment                    Alignment for loading module image
 * @param {guest_image_t *} guest_image         Handle to information regarding the resulted loading of the guest module image
 * @return                                      0 on success, otherwise -1 on error
 */
int vm_load_guest_module(vm_t *vm, const char *module_name, uintptr_t load_address, size_t alignment,
                         guest_image_t *guest_image);
