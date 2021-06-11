<!--
     Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)

     SPDX-License-Identifier: CC-BY-SA-4.0
-->

# libsel4vmmplatsupport

A library containing various VMM utilities and drivers that can be used to construct a Guest VM on a supported platform.
This library makes use of 'libsel4vm' to implement VMM support.
Reference implementations using this library are:
* CAmkES VM (x86) - https://github.com/seL4/camkes-vm
* CAmkES ARM VM (arm) - https://github.com/SEL4PROJ/camkes-arm-vm

For documentation on the libsel4vmmplatsupport interface see [here](docs/)

### Features
* Virtio Support/Drivers
    * Virtio PCI
    * Virtio Console
    * Virtio Net
* Cross VM connection/communication driver
* Guest image loading utilities (e.g. kernel, initramfs)
* `libsel4vm` memory helpers and utilities
* IOPorts management interface

#### Architecture Specific Features

#####  ARM
* VCPU fault handler module
    * HSR/Exception decoding
    * HSR/Exception handler dispatching
    * SMC decoding + handling
* PSCI handlers - through VCPU fault (+SMC handler) interface
* Generic virtual device interfaces
    * Generic access controlled devices - control read/write privileges to specific devices
    * Generic forwarding devices - interfaces for dispatching faults to external handlers
* Guest reboot utilities
* Virtual USB driver
* Guest OS Boot interfaces (for Linux VM's) : Boot VCPU initialisation

##### X86
* ACPI table generation
* PCI device passthrough helpers
* Guest OS Boot interfaces (for Linux VM's) : Boot VCPU initialisation, BIOS boot info structure generation, E820 map generation, VESA initialisation

#### Platform Specific Features

##### Exynos5422
* Virtual Clock device driver (Access Controlled Device)
* Virtual IRQ combiner device driver
* Virtual GPIO device driver
* Virtual Power device driver
* Virtual MCT device driver
* Virtual UART UART device driver

#### TK1
* USB Reboot Hooks

### Potential future features (yet to be implemented)

* Additional Virtio Driver Support
    * e.g Virtio Blk, Virtio RNG, Virtio Balloon
* Block Driver support

#### Architecture Specific Features
##### ARM
* Additional virtual devices for various supported platforms
* Additional platform/board support
##### x86
* Generic virtual device interface (as supported on ARM platforms)
* Additional virtual device support

*Note: This is a consolidated library composed of libraries previously known as (but now deprecated) 'libsel4vmm' (x86),
'libsel4arm-vmm' (arm), libsel4vmmcore and libsel4pci.*
