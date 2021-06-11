<!--
     Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)

     SPDX-License-Identifier: CC-BY-SA-4.0
-->

# libsel4vmmplatsupport docs

See below for usage documentation on various libsel4vmmplatsupport interfaces:

### Common Interfaces
* [sel4vmmplatsupport/device.h](libsel4vmmplatsupport_device.md): Provides a series of datastructures and helpers to manage VMM devices.
* [sel4vmmplatsupport/device_utils.h](libsel4vmmplatsupport_device_utils.md): Provides various helpers to establish different types devices for a given VM instance
* [sel4vmmplatsupport/guest_image.h](libsel4vmmplatsupport_guest_image.md): Provides general utilites to load guest vm images (e.g. kernel, initrd, modules)
* [sel4vmmplatsupport/guest_memory_util.h](libsel4vmmplatsupport_guest_memory_util.md): Provides various utilities and helpers for using the libsel4vm guest memory interface
* [sel4vmmplatsupport/guest_vcpu_util.h](libsel4vmmplatsupport_guest_vcpu_util.md): Provides abstractions and helpers for managing libsel4vm vcpus
* [sel4vmmplatsupport/ioports.h](libsel4vmmplatsupport_ioports.md): Useful abstraction for initialising, registering and handling ioport events for a guest VM instance
* [sel4vmmplatsupport/drivers/cross_vm_connection.h](libsel4vmmplatsupport_cross_vm_connection.md): Facilitates the creation of communication channels between VM's and other components on a seL4-based system
* [sel4vmmplatsupport/drivers/pci.h](libsel4vmmplatsupport_pci.md): Interface presents a VMM PCI Driver, which manages the host's PCI devices, and handles guest OS PCI config space read & writes
* [sel4vmmplatsupport/drivers/pci_helper.h](libsel4vmmplatsupport_pci_helper.md): This interface presents a series of helpers when using the VMM PCI Driver
* [sel4vmmplatsupport/drivers/virtio_con.h](libsel4vmmplatsupport_virtio_con.md): This interface provides the ability to initalise a VMM virtio console driver
* [sel4vmmplatsupport/drivers/virtio_net.h](libsel4vmmplatsupport_virtio_net.md): This interface provides the ability to initalise a VMM virtio net driver

### Architecture Specific Interfaces

#### ARM
* [sel4vmmplatsupport/arch/generic_forward_device.h](libsel4vmmplatsupport_arm_generic_forward_device.md): This interface facilitates the creation of a virtual device used for dispatching faults to external handlers
* [sel4vmmplatsupport/arch/guest_boot_init.h](libsel4vmmplatsupport_arm_guest_boot_init.md): Provides helpers to initialise the booting state of a VM instance
* [sel4vmmplatsupport/arch/guest_reboot,h](libsel4vmmplatsupport_arm_guest_reboot.md): Provides a series of helpers for registering callbacks when rebooting the VMM
* [sel4vmmplatsupport/arch/guest_vcpu_fault.h](libsel4vmmplatsupport_arm_guest_vcpu_fault.md): Provides a module for registering and processing ARM vcpu faults
* [sel4vmmplatsupport/arch/guest_vcpu_util.h](libsel4vmmplatsupport_arm_guest_vcpu_util.md): Provides abstractions and helpers for managing libsel4vm vcpus on an ARM platform
* [sel4vmmplatsupport/arch/vpci.h](libsel4vmmplatsupport_arm_vpci.md): Presents a Virtual PCI driver for ARM-based VM's
* [sel4vmmplatsupport/arch/vusb.h](libsel4vmmplatsupport_arm_vusb.md): Presents a Virtual USB driver for ARM-based VM's
* [sel4vmmplatsupport/arch/ac_device.h](libsel4vmmplatsupport_arm_ac_device.md): Facilitates the creation of generic virtual devices in a VM instance with access control permissions over the devices addressable memory
#### X86
* [sel4vmmplatsupport/arch/acpi.h](libsel4vmmplatsupport_x86_acpi.md): Provides support for generating ACPI table in a guest x86 VM
* [sel4vmmplatsupport/arch/guest_boot_init.h](libsel4vmmplatsupport_x86_guest_boot_init.md): Provides helpers to initialise the booting state of a VM instance
* [sel4vmmplatsupport/arch/drivers/vmm_pci_helper.h](libsel4vmmplatsupport_x86_vmm_pci_helper.md): Interface presents a series of helpers for establishing VMM PCI support on x86 platforms
