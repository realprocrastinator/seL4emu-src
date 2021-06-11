/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>

/***
 * @module guest_ram.h
 * The libsel4vm RAM interface provides us with a set of methods to manage a guest VM's RAM. This involves functions
 * to register, allocate and copy to and from RAM regions.
 */

/**
 * Type signature of ram touch callback function, provided when invoking 'vm_ram_touch'
 * @param {vm_t *} vm               A handle to the VM
 * @param {uintptr_t} guest_addr    Current guest physical address being accessed
 * @param {void *} vmm_vaddr        Virtual address in hosts (vmm) vspace corresponding with the current 'guest_addr'
 * @param {size_t} size             Size of region being currently accessed
 * @param {size_t} offset           Current offset from the base guest physical address supplied to 'vm_ram_touch'
 * @param {void *} cookie           User supplied cookie to pass onto callback
 * @return                          0 on success, -1 on error
 */
typedef int (*ram_touch_callback_fn)(vm_t *vm, uintptr_t guest_addr, void *vmm_vaddr, size_t size, size_t offset,
                                     void *cookie);

/***
 * @function vm_guest_ram_read_callback(vm, guest_addr, vaddr, size, offset, buf)
 * Common guest ram touch callback for reading from a guest address into a user supplied buffer
 * @param {vm_t *} vm               A handle to the VM
 * @param {uintptr_t} guest_addr    Guest physical address to read from
 * @param {void *} vmm_vaddr        Virtual address in hosts (vmm) vspace corresponding with the 'guest_addr'
 * @param {size_t} size             Size of region being currently accessed
 * @param {size_t} offset           Current offset from the base guest physical address supplied to 'vm_ram_touch'
 * @param {void *} cookie           User supplied buffer to store read data into
 * @return                          0 on success, -1 on error
 */
int vm_guest_ram_read_callback(vm_t *vm, uintptr_t guest_addr, void *vaddr, size_t size, size_t offset, void *buf);

/***
 * @function vm_guest_ram_write_callback(vm, guest_addr, vaddr, size, offset, buf)
 * Common guest ram touch callback for writing a user supplied buffer into a guest address
 * @param {vm_t *} vm               A handle to the VM
 * @param {uintptr_t} guest_addr    Guest physical address to write to
 * @param {void *} vmm_vaddr        Virtual address in hosts (vmm) vspace corresponding with the 'guest_addr'
 * @param {size_t} size             Size of region being currently accessed
 * @param {size_t} offset           Current offset from the base guest physical address supplied to 'vm_ram_touch'
 * @param {void *} cookie           User supplied buffer to write data from
 * @return                          0 on success, -1 on error
 */
int vm_guest_ram_write_callback(vm_t *vm, uintptr_t guest_addr, void *vaddr, size_t size, size_t offset, void *buf);

/***
 * @function vm_ram_touch(vm, addr, size, touch_callback, cookie)
 * Touch a series of pages in the guest vm and invoke a callback for each page accessed
 * @param {vm_t *} vm                       A handle to the VM
 * @param {uintptr_t} addr                  Address to access in the guest vm
 * @param {size_t} size                     Size of memory region to access
 * @param {ram_touch_callback_fn} callback  Callback to invoke on each page access
 * @param {void *} cookie                   User data to pass onto callback
 * @return                                  0 on success, -1 on error
 */
int vm_ram_touch(vm_t *vm, uintptr_t addr, size_t size, ram_touch_callback_fn touch_callback, void *cookie);

/***
 * @function vm_ram_find_largest_free_region(vm, addr, size)
 * Find the largest free ram region
 * @param {vm_t *} vm               A handle to the VM
 * @param {uintptr_t *} addr        Pointer to be set with largest region address
 * @param {size_t *} size           Pointer to be set with largest region size
 * @return                          -1 on failure, otherwise 0 for success
 */
int vm_ram_find_largest_free_region(vm_t *vm, uintptr_t *addr, size_t *size);

/***
 * @function vm_ram_register(vm, bytes)
 * Reserve a region of memory for RAM in the guest VM
 * @param {vm_t *} vm           A handle to the VM
 * @param {size_t} bytes        Size of RAM region to allocate
 * @return                      Starting address of registered ram region
 */
uintptr_t vm_ram_register(vm_t *vm, size_t bytes);

/***
 * @function vm_ram_register_at(vm, start, bytes, untyped)
 * Reserve a region of memory for RAM in the guest VM at a starting guest physical address
 * @param {vm_t *} vm           A handle to the VM that ram needs to be allocated for
 * @param {uintptr_t} start     Starting guest physical address of the ram region being allocated
 * @param {size_t} size         The size of the RAM region to be allocated
 * @param {bool} untyped        Allocate RAM frames such that it uses untyped memory
 * @return                      0 on success, -1 on error
 */
int vm_ram_register_at(vm_t *vm, uintptr_t start, size_t bytes, bool untyped);

/***
 * @function vm_ram_register_at(vm, start, bytes, untyped)
 * Reserve a region of memory for RAM in the guest VM at a starting guest physical address with a custom memory iterator
 * @param {vm_t *} vm                           A handle to the VM that ram needs to be allocated for
 * @param {uintptr_t} start                     Starting guest physical address of the ram region being allocated
 * @param {size_t} size                         The size of the RAM region to be allocated
 * @param {memory_map_iterator_fn} map_iterator Iterator function
 * @param {void *} cookie                       Iterator cookie
 * @return                      0 on success, -1 on error
 */
int vm_ram_register_at_custom_iterator(vm_t *vm, uintptr_t start, size_t bytes, memory_map_iterator_fn map_iterator,
                                       void *cookie);

/***
 * @function vm_ram_mark_allocated(vm, start, bytes)
 * Mark a registered region of RAM as allocated
 * @param {vm_t *} vm               A handle to the VM
 * @param {uintptr_t} start         Starting address of guest ram region
 * @param {size_t} bytes            Size of RAM region
 */
void vm_ram_mark_allocated(vm_t *vm, uintptr_t start, size_t bytes);

/***
 * @function vm_ram_allocate(vm, bytes)
 * Allocate a region of registered ram
 * @param {vm_t *} vm           A handle to the VM
 * @param {size_t} bytes        Size of allocation
 * @return                      Starting address of allocated ram region
 */
uintptr_t vm_ram_allocate(vm_t *vm, size_t bytes);

/***
 * @function vm_ram_free(vm, start, bytes)
 * Free a RAM a previously allocated RAM region
 * @param {vm_t *} vm           A handle to the VM that ram needs to be free'd for
 * @param {uintptr_t} start     Starting guest physical address of the ram region being free'd
 * @param {size_t} size         The size of the RAM region to be free'd
 */
void vm_ram_free(vm_t *vm, uintptr_t start, size_t bytes);
