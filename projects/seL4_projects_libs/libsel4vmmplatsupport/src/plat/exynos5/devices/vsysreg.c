/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>

#include <sel4vmmplatsupport/plat/vsysreg.h>
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/plat/devices.h>

struct sysreg_priv {
    struct dma *dma_list;
    vm_t *vm;
    void *regs;
};

static memory_fault_result_t handle_vsysreg_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                                  void *cookie)
{
    struct device *dev = (struct device *)cookie;
    struct sysreg_priv *sysreg_data = (struct sysreg_priv *)dev->priv;
    volatile uint32_t *reg;
    int offset;

    /* Gather fault information */
    offset = fault_addr - dev->pstart;
    reg = (uint32_t *)(sysreg_data->regs + offset);
    /* Handle the fault */
    reg = (volatile uint32_t *)(sysreg_data->regs + offset);
    if (is_vcpu_read_fault(vcpu)) {
        set_vcpu_fault_data(vcpu, *reg);
        ZF_LOGD("[%s] pc0x%x| r0x%x:0x%x\n", dev->name, get_vcpu_fault_ip(vcpu),
                fault_addr, get_vcpu_fault_data(vcpu));
    } else {
        ZF_LOGD("[%s] pc0x%x| w0x%x:0x%x\n", dev->name, get_vcpu_fault_ip(vcpu),
                fault_addr, get_vcpu_fault_data(vcpu));
        *reg = emulate_vcpu_fault(vcpu, *reg);
    }
    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}

const struct device dev_sysreg = {
    .name = "sysreg",
    .pstart = SYSREG_PADDR,
    .size = 0x1000,
    .priv = NULL
};

int vm_install_vsysreg(vm_t *vm)
{
    struct sysreg_priv *sysreg_data;
    struct device *d;
    int err;

    d = (struct device *)calloc(1, sizeof(struct device));
    memcpy(d, &dev_sysreg, sizeof(struct device));

    /* Initialise the virtual device */
    sysreg_data = calloc(1, sizeof(struct sysreg_priv));
    if (sysreg_data == NULL) {
        assert(sysreg_data);
        free(d);
        return -1;
    }
    sysreg_data->vm = vm;

    sysreg_data->regs = ps_io_map(&vm->io_ops->io_mapper, d->pstart, PAGE_SIZE_4K, 0, PS_MEM_NORMAL);
    if (sysreg_data->regs == NULL) {
        free(d);
        free(sysreg_data);
        return -1;
    }

    d->priv = sysreg_data;

    vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, d->pstart, d->size,
                                                                handle_vsysreg_fault, (void *)d);
    if (!reservation) {
        free(d);
        free(sysreg_data);
        return -1;
    }

    return 0;
}
