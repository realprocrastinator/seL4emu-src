/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>

#include <sel4vmmplatsupport/arch/generic_forward_device.h>
#include <sel4vmmplatsupport/device.h>

struct gf_device_priv {
    struct generic_forward_cfg cfg;
};

static memory_fault_result_t handle_gf_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                             void *cookie)
{
    struct device *dev = (struct device *)cookie;
    struct gf_device_priv *gf_device_priv = (struct gf_device_priv *)dev->priv;

    /* Gather fault information */
    uint32_t offset = fault_addr - dev->pstart;
    if (offset >= dev->size) {
        ZF_LOGF("Fault on address not supported by this handler");
    }

    /* Dispatch to external fault handler */
    if (is_vcpu_read_fault(vcpu)) {
        if (gf_device_priv->cfg.read_fn == NULL) {
            ZF_LOGD("No read function provided");
            set_vcpu_fault_data(vcpu, 0);
        } else {
            set_vcpu_fault_data(vcpu, gf_device_priv->cfg.read_fn(offset));
        }
    } else  {
        if (gf_device_priv->cfg.write_fn == NULL) {
            ZF_LOGD("No write function provided");
        } else {
            gf_device_priv->cfg.write_fn(offset, get_vcpu_fault_data(vcpu));
        }
    }

    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}


int vm_install_generic_forward_device(vm_t *vm, const struct device *d,
                                      struct generic_forward_cfg cfg)
{
    struct gf_device_priv *gf_device_priv;
    struct device *dev;
    int err;

    dev = (struct device *)calloc(1, sizeof(struct device));
    if (!dev) {
        return -1;
    }
    memcpy(dev, d, sizeof(struct device));

    /* initialise private data */
    gf_device_priv = (struct gf_device_priv *)calloc(1, sizeof(*gf_device_priv));
    if (gf_device_priv == NULL) {
        ZF_LOGE("error calloc returned null");
        free(dev);
        return -1;
    }
    gf_device_priv->cfg = cfg;

    /* Add the device */
    dev->priv = gf_device_priv;

    vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, dev->pstart, dev->size,
                                                                handle_gf_fault, (void *)dev);
    if (!reservation) {
        free(dev);
        free(gf_device_priv);
        return -1;
    }

    return 0;
}
