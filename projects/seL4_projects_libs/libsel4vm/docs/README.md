<!--
     Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)

     SPDX-License-Identifier: CC-BY-SA-4.0
-->

# libsel4vm docs

See below for usage documentation on various libsel4vm interfaces:

### Common Interfaces
* [sel4vm/boot.h](libsel4vm_boot.md): An interface for creating, initialising and configuring VM instances
* [sel4vm/guest_irq_controller.h](libsel4vm_guest_irq_controller.md): Abstractions around initialising a guest VM IRQ controller
* [sel4vm/guest_memory_helpers.h](libsel4vm_guest_memory_helpers.md): Helpers for using the guest memory interface
* [sel4vm/guest_vcpu_fault.h](libsel4vm_guest_vcpu_fault.md): Useful methods to query and configure vcpu objects that have faulted during execution
* [sel4vm/guest_vm.h](libsel4vm_guest_vm.md): Provides base definitions of the guest vm datastructure and primitives to run the VM instance
* [sel4vm/guest_iospace.h](libsel4vm_guest_iospace.md):  Enables the registration and management of a guest VM's IO Space
* [sel4vm/guest_memory.h](libsel4vm_guest_memory.md): Useful abstractions to manage your guest VM's physical address space
* [sel4vm/guest_ram.h](libsel4vm_guest_ram.md): A set of methods to manage, register, allocate and copy to/from a guest VM's RAM
* [sel4vm/guest_vm_util.h](libsel4vm_guest_vm_util.md): A set of utilties to query a guest vm instance

### Architecture Specific Interfaces

#### ARM
* [sel4vm/arch/guest_arm_context.h](libsel4vm_guest_arm_context.md): Provides a set of useful getters and setters on ARM vcpu thread contexts
* [sel4vm/arch/guest_vm_arch.h](libsel4vm_arm_guest_vm.md): Provide definitions of the arm guest vm datastructures and primitives to configure the VM instance
#### X86
* [sel4vm/arch/guest_x86_context.h](libsel4vm_guest_x86_context.md): Provides a set of useful getters and setters on x86 vcpu thread contexts
* [sel4vm/arch/guest_vm_arch.h](libsel4vm_x86_guest_vm.md): Provide definitions of the x86 guest vm datastructures and primitives to configure the VM instance
* [sel4vm/arch/vmcall.h](libsel4vm_x86_vmcall.md): Methods for registering and managing vmcall instruction handlers
* [sel4vm/arch/ioports.h](libsel4vm_x86_ioports.md): Abstractions for initialising, registering and handling ioport events
