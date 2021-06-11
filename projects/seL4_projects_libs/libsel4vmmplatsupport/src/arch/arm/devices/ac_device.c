/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/arch/ac_device.h>

struct gac_device_priv {
    void *regs;
    void *mask;
    size_t mask_size;
    enum vacdev_action action;
};

static memory_fault_result_t handle_gac_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                              void *cookie)
{
    struct device *dev = (struct device *)cookie;
    struct gac_device_priv *gac_device_priv = (struct gac_device_priv *)dev->priv;
    volatile uintptr_t *reg;
    int offset;

    /* Gather fault information */
    offset = fault_addr - dev->pstart;
    reg = (volatile seL4_Word *)(gac_device_priv->regs + offset);

    if (is_vcpu_read_fault(vcpu)) {
        set_vcpu_fault_data(vcpu, *reg);
    } else if (offset < gac_device_priv->mask_size) {
        seL4_Word mask, result;
        assert((offset & MASK(2)) == 0);
        mask = *(seL4_Word *)(gac_device_priv->mask + offset);

        result = emulate_vcpu_fault(vcpu, *reg);
        /* Check for a change that involves denied access */
        if ((result ^ *reg) & ~mask) {
            /* Report */
            switch (gac_device_priv->action) {
            case VACDEV_REPORT_AND_MASK:
            case VACDEV_REPORT_ONLY:
                printf("[ac/%s] pc %p | access violation: bits %p @ %p\n",
                       dev->name, (void *) get_vcpu_fault_ip(vcpu),
                       (void *)((result ^ *reg) & ~mask),
                       (void *) fault_addr);
            default:
                break;
            }
            /* Mask */
            switch (gac_device_priv->action) {
            case VACDEV_REPORT_AND_MASK:
            case VACDEV_MASK_ONLY:
                result = (result & mask) | (*reg & ~mask);
                break;
            case VACDEV_REPORT_ONLY:
                break;
            default:
                assert(!"Invalid action");
            }
        }
        *reg = result;
    }

    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}


int vm_install_generic_ac_device(vm_t *vm, const struct device *d, void *mask,
                                 size_t mask_size, enum vacdev_action action)
{
    struct gac_device_priv *gac_device_priv;
    struct device *dev;
    vspace_t *vmm_vspace;
    vmm_vspace = &vm->mem.vmm_vspace;
    int err;

    dev = (struct device *)calloc(1, sizeof(struct device));
    if (!dev) {
        return -1;
    }
    memcpy(dev, d, sizeof(struct device));

    /* initialise private data */
    gac_device_priv = (struct gac_device_priv *)calloc(1, sizeof(*gac_device_priv));
    if (gac_device_priv == NULL) {
        free(dev);
        return -1;
    }
    gac_device_priv->mask = mask;
    gac_device_priv->mask_size = mask_size;
    gac_device_priv->action = action;

    /* Map the device */
    gac_device_priv->regs = ps_io_map(&vm->io_ops->io_mapper, d->pstart, PAGE_SIZE_4K, 0, PS_MEM_NORMAL);

    if (gac_device_priv->regs == NULL) {
        free(dev);
        free(gac_device_priv);
        return -1;
    }

    /* Add the device */
    dev->priv = gac_device_priv;

    vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, dev->pstart, dev->size,
                                                                handle_gac_fault, (void *)dev);
    if (!reservation) {
        free(dev);
        free(gac_device_priv);
        return -1;
    }

    return 0;
}

