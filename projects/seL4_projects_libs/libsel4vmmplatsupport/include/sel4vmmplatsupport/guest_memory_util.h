/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/***
 * @module guest_memory_util.h
 * The guest memory util interface provides various utilities and helpers for using the libsel4vm guest memory
 * interface. The methods in the interface cover common usage patterns when managing memory for a VM instance.
 */

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_memory.h>

/***
 * @function create_allocated_reservation_frame(vm, addr, rights, alloc_fault_callback, alloc_fault_cookie)
 * Create and map a reservation for a vka allocated frame. The allocated frame is mapped in both the vm and vmm vspace
 * @param {vm_t *} vm                                       A handle to the VM
 * @param {uintptr_t} addr                                  Address of emulated frame
 * @param {seL4_CapRights_t} rights                         Rights for mapping the allocated frame into the vm's vspace
 * @param {memory_fault_callback_fn} alloc_fault_callback   Fault callback for allocated frame
 * @param {void *} alloc_fault_cookie                       Cookie for fault callback
 * @return                                                  Address of allocated frame in vmm vspace
 */
void *create_allocated_reservation_frame(vm_t *vm, uintptr_t addr, seL4_CapRights_t rights,
                                         memory_fault_callback_fn alloc_fault_callback, void *alloc_fault_cookie);
/***
 * @function create_device_reservation_frame(vm, addr, rights, fault_callback, fault_cookie)
 * Create and map a reservation for a device frame. The device frame is mapped in both the vm and vmm vspace
 * @param {vm_t *} vm                                       A handle to the VM
 * @param {uintptr_t} addr                                  Address of emulated frame
 * @param {seL4_CapRights_t} rights                         Rights for mapping the device frame into the vm's vspace
 * @param {memory_fault_callback_fn} fault_callback         Fault callback for the frame
 * @param {void *} fault_cookie                             Cookie for fault callback
 * @return                                                  Address of device frame in vmm vspace
 */
void *create_device_reservation_frame(vm_t *vm, uintptr_t addr, seL4_CapRights_t rights,
                                      memory_fault_callback_fn fault_callback, void *fault_cookie);
/***
 * @function map_ut_alloc_reservation_with_base_paddr(vm, paddr, reservation)
 * Map a guest reservation backed with untyped frames allocated from a base paddr
 * @param {vm_t *} vm                               A handle to the VM
 * @param {uintptr_t} paddr                         Base paddr to allocate from
 * @param {vm_memory_reservation_t *} reservation   Pointer to reservation object being mapped
 * @return                                          -1 on failure otherwise 0 for success
 */
int map_ut_alloc_reservation_with_base_paddr(vm_t *vm, uintptr_t paddr,
                                             vm_memory_reservation_t *reservation);

/***
 * @function map_ut_alloc_reservation(vm, reservation)
 * Map a guest reservation backed with untyped frames
 * @param {vm_t *} vm                               A handle to the VM
 * @param {vm_memory_reservation_t *} reservation   Pointer to reservation object being mapped
 * @return                                          -1 on failure otherwise 0 for success
 */
int map_ut_alloc_reservation(vm_t *vm, vm_memory_reservation_t *reservation);

/***
 * @function map_frame_alloc_reservation(vm, reservation)
 * Map a guest reservation backed with free vka frames
 * @param {vm_t *} vm                                   A handle to the VM
 * @param {vm_memory_reservation_t *} reservation       Pointer to reservation object being mapped
 * @return                                              -1 on failure otherwise 0 for success
 */
int map_frame_alloc_reservation(vm_t *vm, vm_memory_reservation_t *reservation);

/***
 * @function map_maybe_device_reservation(vm, reservation)
 * Map a guest reservation backed with device frames
 * @param {vm_t *} vm                                   A handle to the VM
 * @param {vm_memory_reservation_t *} reservation       Pointer to reservation object being mapped
 * @return                                              -1 on failure otherwise 0 for success
 */
int map_maybe_device_reservation(vm_t *vm, vm_memory_reservation_t *reservation);
