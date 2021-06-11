/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/***
 * @module guest_boot_init.h
 * The libsel4vmmplatsupport arm guest boot init interface provides helpers to initialise the booting state of
 * a VM instance. This currently only targets booting a Linux guest OS.
 */

#include <sel4/sel4.h>

#include <sel4vm/guest_vm.h>

/***
 * @function vcpu_set_bootargs(vcpu, pc, mach_type, atags)
 * Set the boot args and pc for the VM.
 * For linux:
 *   r0 -> 0
 *   r1 -> MACH_TYPE  (e.g #4151 for EXYNOS5410 eval. platform smdk5410)
 *   r2 -> atags/dtb address
 * @param {vm_vcpu_t *} vcpu        A handle to the boot VCPU
 * @param {seL4_Word} pc            The initial PC for the VM
 * @param {seL4_Word} mach_type     Linux specific machine ID see http://www.arm.linux.org.uk/developer/machines/
 * @param {seL4_Word} atags         Linux specific IPA of atags. Can also be substituted with dtb address
 * @return                          0 on success, otherwise -1 for failure
 */
int vcpu_set_bootargs(vm_vcpu_t *vcpu, seL4_Word pc, seL4_Word mach_type, seL4_Word atags);
