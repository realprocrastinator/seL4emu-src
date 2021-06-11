/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

/***
 * @module guest_boot_init.h
 * The libsel4vmmplatsupport x86 guest boot init interface provides helpers to initialise the booting state of
 * a VM instance. This currently only targets booting a Linux guest OS.
 */

#include <stdint.h>

#include <sel4vmmplatsupport/guest_image.h>

/***
 * @function vmm_plat_init_guest_boot_structure(vm, cmdline, guest_kernel_image, guest_ramdisk_image, guest_boot_info_addr)
 * Establish the necessary BIOS boot structures to initialise and boot a guest Linux OS. This includes the creation of a BIOS
 * boot info structure, an e820 map and ACPI tables.
 * @param {vm_t *} vm                                   A handle to the guest VM
 * @param {const char *} cmdline                        The guest Linux boot commandline
 * @param {guest_kernel_image_t} guest_kernel_image     Guest kernel image (preloaded into the VM's memory)
 * @param {guest_image_t} guest_ramdisk_image           Guest ramdisk image (preloaded into the VM's memory)
 * @param {uintptr_t *} guest_boot_info_addr            Resulting address of loaded generated guest boot info structure
 *                                                      (in guest physical address space)
 * @return                                              0 for success, -1 for error
 */
int vmm_plat_init_guest_boot_structure(vm_t *vm, const char *cmdline,
                                       guest_kernel_image_t guest_kernel_image, guest_image_t guest_ramdisk_image,
                                       uintptr_t *guest_boot_info_addr);

/***
 * @function vmm_plat_init_guest_thread_state(vcpu, guest_entry_addr, guest_boot_info_addr)
 * Initialise the booting state of a guest VM, establishing the necessary thread state to launch a guest Linux OS
 * @param {vm_vcpu_t *} vcpu                A handle to the boot vcpu
 * @param {uintptr_t} guest_entry_addr      Address of VM entry point (often entry point defined in kernel elf image)
 * @param {uintptr_t} guest_boot_info_addr  Address of loaded guest boot info structure
 * @return                                  0 for success, -1 for error
 */
int vmm_plat_init_guest_thread_state(vm_vcpu_t *vcpu, uintptr_t guest_entry_addr,
                                     uintptr_t guest_boot_info_addr);
