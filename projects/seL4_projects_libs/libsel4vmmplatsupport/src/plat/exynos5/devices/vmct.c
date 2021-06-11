/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>

#include <sel4vmmplatsupport/plat/vmct.h>
#include <sel4vmmplatsupport/plat/devices.h>

#define GWSTAT_TCON          (1U << 16)
#define GWSTAT_COMP3_ADD_INC (1U << 14)
#define GWSTAT_COMP3H        (1U << 13)
#define GWSTAT_COMP3L        (1U << 12)
#define GWSTAT_COMP2_ADD_INC (1U << 10)
#define GWSTAT_COMP2H        (1U << 9)
#define GWSTAT_COMP2L        (1U << 8)
#define GWSTAT_COMP1_ADD_INC (1U << 6)
#define GWSTAT_COMP1H        (1U << 5)
#define GWSTAT_COMP1L        (1U << 4)
#define GWSTAT_COMP0_ADD_INC (1U << 2)
#define GWSTAT_COMP0H        (1U << 1)
#define GWSTAT_COMP0L        (1U << 0)


struct vmct_priv {
    uint32_t wstat;
    uint32_t cnt_wstat;
    uint32_t lwstat[4];
};

static inline struct vmct_priv *vmct_get_priv(void *priv)
{
    assert(priv);
    return (struct vmct_priv *)priv;
}


static memory_fault_result_t handle_vmct_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                               void *cookie)
{
    struct vmct_priv *mct_priv;
    int offset;
    uint32_t mask;
    struct device *dev = (struct device *)cookie;

    mct_priv = vmct_get_priv(dev->priv);

    /* Gather fault information */
    offset = fault_addr - dev->pstart;
    mask = get_vcpu_fault_data_mask(vcpu);
    /* Handle the fault */
    if (offset < 0x300) {
        /*** Global ***/
        uint32_t *wstat;
        uint32_t *cnt_wstat;
        wstat = &mct_priv->wstat;
        cnt_wstat = &mct_priv->cnt_wstat;
        if (!is_vcpu_read_fault(vcpu)) {
            if (offset >= 0x100 && offset < 0x108) {
                /* Count registers */
                *cnt_wstat |= (1 << (offset - 0x100) / 4);
            } else if (offset == 0x110) {
                *cnt_wstat &= ~(get_vcpu_fault_data(vcpu) & mask);
            } else if (offset >= 0x200 && offset < 0x244) {
                /* compare registers */
                *wstat |= (1 << (offset - 0x200) / 4);
            } else if (offset == 0x24C) {
                /* Write status */
                *wstat &= ~(get_vcpu_fault_data(vcpu) & mask);
            } else {
                ZF_LOGD("global MCT fault on unknown offset 0x%x\n", offset);
            }
        } else {
            /* read fault */
            if (offset == 0x110) {
                set_vcpu_fault_data(vcpu, *cnt_wstat);
            } else if (offset == 0x24C) {
                set_vcpu_fault_data(vcpu, *wstat);
            } else {
                set_vcpu_fault_data(vcpu, 0);
            }
        }
    } else {
        /*** Local ***/
        int timer = (offset - 0x300) / 0x100;
        int loffset = (offset - 0x300) - (timer * 0x100);
        uint32_t *wstat = &mct_priv->lwstat[timer];
        if (is_vcpu_read_fault(vcpu)) {
            if (loffset == 0x0) {        /* tcompl */
                *wstat |= (1 << 1);
            } else if (loffset == 0x8) {  /* tcomph */
                *wstat |= (1 << 1);
            } else if (loffset == 0x20) { /* tcon */
                *wstat |= (1 << 3);
            } else if (loffset == 0x34) { /* int_en */
                /* Do nothing */
            } else if (loffset == 0x40) { /* wstat */
                *wstat &= ~(get_vcpu_fault_data(vcpu) & mask);
            } else {
                ZF_LOGD("local MCT fault on unknown offset 0x%x\n", offset);
            }
        } else {
            /* read fault */
            if (loffset == 0x40) {
                set_vcpu_fault_data(vcpu, *wstat);
            } else {
                set_vcpu_fault_data(vcpu, 0);
            }
        }
    }
    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}

const struct device dev_vmct_timer = {
    .name = "mct",

    .pstart = MCT_ADDR,

    .size = 0x1000,
    .priv = NULL
};


int vm_install_vmct(vm_t *vm)
{
    struct vmct_priv *vmct_data;
    struct device *d;
    int err;

    d = (struct device *)calloc(1, sizeof(struct device));
    if (!d) {
        return -1;
    }
    memcpy(d, &dev_vmct_timer, sizeof(struct device));
    /* Initialise the virtual device */
    vmct_data = calloc(1, sizeof(struct vmct_priv));
    if (vmct_data == NULL) {
        assert(vmct_data);
        return -1;
    }
    d->priv = vmct_data;
    vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, d->pstart, d->size,
                                                                handle_vmct_fault, (void *)d);
    if (!reservation) {
        free(d);
        free(vmct_data);
        return -1;
    }
    return 0;
}
