/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/*
 * DesignWare eMMC
 */

#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>

#include <sel4vmmplatsupport/guest_memory_util.h>
#include <sel4vmmplatsupport/plat/vsdhc.h>
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/plat/devices.h>

#define DWEMMC_DBADDR_OFFSET    0x088
#define DWEMMC_DSCADDR_OFFSET   0x094
#define DWEMMC_BUFADDR_OFFSET   0x098

struct sdhc_priv {
    /* The VM associated with this device */
    vm_t *vm;
    /* Physical registers of the SDHC */
    void *regs;
    /* Residual for 64 bit atomic access to FIFO */
    uint32_t a64;
};

static memory_fault_result_t handle_sdhc_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                               void *cookie)
{
    struct device *d = (struct device *)cookie;
    struct sdhc_priv *sdhc_data = (struct sdhc_priv *)d->priv;
    volatile uint32_t *reg;
    int offset;

    /* Gather fault information */
    offset = fault_addr - d->pstart;
    reg = (uint32_t *)(sdhc_data->regs + offset);
    /* Handle the fault */
    reg = (volatile uint32_t *)(sdhc_data->regs + offset);
    if (is_vcpu_read_fault(vcpu)) {
        if (fault_length == sizeof(uint64_t)) {
            if (offset & 0x4) {
                /* Unaligned access: report residual */
                set_vcpu_fault_data(vcpu, sdhc_data->a64);
            } else {
                /* Aligned access: Read in and store residual */
                uint64_t v;
                v = *(volatile uint64_t *)reg;
                set_vcpu_fault_data(vcpu, v);
                sdhc_data->a64 = v >> 32;
            }
        } else {
            assert(fault_length == sizeof(seL4_Word));
            set_vcpu_fault_data(vcpu, *reg);
        }
        ZF_LOGD("[%s] pc0x%x| r0x%x:0x%x\n", d->name, get_vcpu_fault_ip(vcpu),
                fault_addr, get_vcpu_fault_data(vcpu));
    } else {
        switch (offset & ~0x3) {
        case DWEMMC_DBADDR_OFFSET:
        case DWEMMC_DSCADDR_OFFSET:
        case DWEMMC_BUFADDR_OFFSET:
            printf("[%s] Restricting DMA access offset 0x%x\n", d->name, offset);
            break;
        default:
            if (fault_length == sizeof(uint64_t)) {
                if (offset & 0x4) {
                    /* Unaligned acces: store data and residual */
                    uint64_t v;
                    v = ((uint64_t)get_vcpu_fault_data(vcpu) << 32) | sdhc_data->a64;
                    *(volatile uint64_t *)reg = v;
                } else {
                    /* Aligned access: record residual */
                    sdhc_data->a64 = get_vcpu_fault_data(vcpu);
                }
            } else {
                assert(fault_length == sizeof(seL4_Word));
                *reg = get_vcpu_fault_data(vcpu);
            }
        }

        ZF_LOGD("[%s] pc0x%x| w0x%x:0x%x\n", d->name, get_vcpu_fault_ip(vcpu),
                fault_addr, get_vcpu_fault_data(vcpu));
    }
    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}


const struct device dev_msh0 = {
    .name = "MSH0",
    .pstart = MSH0_PADDR,
    .size = 0x1000,
    .priv = NULL
};

const struct device dev_msh2 = {
    .name = "MSH2",
    .pstart = MSH2_PADDR,
    .size = 0x1000,
    .priv = NULL
};

static int vm_install_nodma_sdhc(vm_t *vm, int idx)
{
    struct sdhc_priv *sdhc_data;
    struct device *d;
    int err;
    d = calloc(1, sizeof(struct device));
    if (!d) {
        return -1;
    }
    switch (idx) {
    case 0:
        *d = dev_msh0;
        break;
    case 2:
        *d = dev_msh2;
        break;
    default:
        assert(0);
        return -1;
    }

    /* Initialise the virtual device */
    sdhc_data = calloc(1, sizeof(struct sdhc_priv));
    if (sdhc_data == NULL) {
        assert(sdhc_data);
        return -1;
    }
    sdhc_data->vm = vm;
    sdhc_data->regs = create_device_reservation_frame(vm, d->pstart, seL4_CanRead,
                                                      handle_sdhc_fault, (void *)d);
    if (sdhc_data->regs == NULL) {
        assert(sdhc_data->regs);
        return -1;
    }
    d->priv = sdhc_data;
    return 0;
}

int vm_install_nodma_sdhc0(vm_t *vm)
{
    return vm_install_nodma_sdhc(vm, 0);
}

int vm_install_nodma_sdhc2(vm_t *vm)
{
    return vm_install_nodma_sdhc(vm, 2);
}
