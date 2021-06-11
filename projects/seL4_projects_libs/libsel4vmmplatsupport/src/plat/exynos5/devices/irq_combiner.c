/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_memory.h>
#include <sel4vm/boot.h>
#include <sel4vm/guest_vcpu_fault.h>

#include <sel4vmmplatsupport/guest_memory_util.h>
#include <sel4vmmplatsupport/plat/device_map.h>
#include <sel4vmmplatsupport/plat/irq_combiner.h>

#include "irq_combiner.h"
#include <stdlib.h>
#include <string.h>
#include <platsupport/irq_combiner.h>

struct combiner_gmap {
    uint32_t enable_set;
    uint32_t enable_clr;
    uint32_t status;
    uint32_t masked_status;
};



struct irq_combiner_map {
    struct combiner_gmap g[8];
    uint32_t res[32];
    uint32_t pending;
};



struct irq_group_data {
    combiner_irq_handler_fn cb;
    void *priv;
};

struct combiner_data {
    irq_combiner_t pcombiner;
    struct irq_group_data *data[32];
};

/* For virtualisation */
struct virq_combiner {
    /* What the VM reads */
    struct irq_combiner_map *vregs;
};

struct combiner_data _combiner;

static inline struct virq_combiner *vcombiner_priv_get_vcombiner(void *priv)
{
    return (struct virq_combiner *)priv;
}

void vm_combiner_irq_handler(vm_t *vm, int irq)
{
    struct combiner_data *combiner = &_combiner;
    struct combiner_irq *cirq;
    uint32_t imsr;
    int i, g;

    /* Decode group and index */
    g = irq - 32;
    imsr = irq_combiner_group_pending(&combiner->pcombiner, g);
    i = __builtin_ctz(imsr);
    /* Disable the IRQ */
    irq_combiner_enable_irq(&combiner->pcombiner, COMBINER_IRQ(g, i));

    /* Allocate a combiner IRQ structure */
    cirq = (struct combiner_irq *)calloc(1, sizeof(*cirq));
    assert(cirq);
    if (!cirq) {
        printf("No memory to allocate combiner IRQ\n");
        return;
    }
    cirq->priv = combiner->data[g][i].priv;
    cirq->index = i;
    cirq->group = g;
    cirq->combiner_priv = combiner;

    /* Forward the IRQ */
    combiner->data[g][i].cb(cirq);
}

void combiner_irq_ack(struct combiner_irq *cirq)
{
    struct combiner_data *combiner;
    int g, i;
    assert(cirq->combiner_priv);
    combiner = (struct combiner_data *)cirq->combiner_priv;
    g = cirq->group;
    i = cirq->index;
    /* Re-enable the IRQ */
    irq_combiner_enable_irq(&combiner->pcombiner, COMBINER_IRQ(g, i));
    free(cirq);
}


int vmm_register_combiner_irq(int group, int idx, combiner_irq_handler_fn cb, void *priv)
{
    struct combiner_data *combiner = &_combiner;
    assert(0 <= group && group < 32);
    assert(0 <= idx && idx < 8);

    /* If no IRQ's for this group yet, setup the group */
    if (combiner->data[group] == NULL) {
        void *addr;

        addr = calloc(1, sizeof(struct irq_group_data) * 8);
        assert(addr);
        if (addr == NULL) {
            return -1;
        }

        combiner->data[group] = (struct irq_group_data *)addr;

        ZF_LOGD("Registered combiner IRQ (%d, %d)\n", group, idx);
    }

    /* Register the callback */
    combiner->data[group][idx].cb = cb;
    combiner->data[group][idx].priv = priv;

    /* Enable the irq */
    irq_combiner_enable_irq(&combiner->pcombiner, COMBINER_IRQ(group, idx));
    return 0;
}

static memory_fault_result_t vcombiner_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                             void *cookie)
{
    int err;
    memory_fault_result_t ret;
    struct virq_combiner *vcombiner;
    int offset;
    int gidx;
    uint32_t mask;
    uint32_t *reg;
    struct device *d;

    d = (struct device *)cookie;
    assert(d->priv);

    vcombiner = vcombiner_priv_get_vcombiner(d->priv);
    mask = get_vcpu_fault_data_mask(vcpu);
    offset = fault_addr - d->pstart;
    reg = (uint32_t *)((uintptr_t)vcombiner->vregs + offset);
    gidx = offset / sizeof(struct combiner_gmap);
    assert(offset >= 0 && offset < sizeof(*vcombiner->vregs));

    ret = FAULT_HANDLED;
    if (offset == 0x100) {
        ZF_LOGD("Fault on group pending register\n");
    } else if (offset < 0x80) {
        uint32_t data;
        int group, index;
        (void)group;
        (void)index;
        switch (offset / 4 % 4) {
        case 0:
            data = get_vcpu_fault_data(vcpu);
            data &= mask;
            data &= ~(*reg);
            while (data) {
                int i;
                i = CTZ(data);
                data &= ~(1U << i);
                group = gidx * 4 + i / 8;
                index = i % 8;
                ZF_LOGD("enable IRQ %d.%d (%d)\n", group, index, group + 32);
            }
            ret = FAULT_IGNORE;
            break;
        case 1:
            data = get_vcpu_fault_data(vcpu);
            data &= mask;
            data &= *reg;
            while (data) {
                int i;
                i = CTZ(data);
                data &= ~(1U << i);
                group = gidx * 4 + i / 8;
                index = i % 8;
                ZF_LOGD("disable IRQ %d.%d (%d)\n", group, index, group + 32);
            }
            ret = FAULT_IGNORE;
            break;
        case 2:
        case 3:
        /* Read only registers */
        default:
            ZF_LOGD("Error handling register access at offset 0x%x\n", offset);
        }
    } else {
        ZF_LOGD("Unknown register access at offset 0%x\n", offset);
    }
    if (err) {
        ret = FAULT_ERROR;
    }
    advance_vcpu_fault(vcpu);
    return ret;
}



const struct device dev_irq_combiner = {
    .name = "irq.combiner",
    .pstart = IRQ_COMBINER_PADDR,
    .size = 0x1000,
    .priv = NULL
};

int vm_install_vcombiner(vm_t *vm)
{
    struct device *combiner;
    struct virq_combiner *vcombiner;
    void *addr;
    int err;

    err = irq_combiner_init(IRQ_COMBINER0, vm->io_ops, &_combiner.pcombiner);
    if (err) {
        return -1;
    }

    /* Distributor */
    combiner = (struct device *)calloc(1, sizeof(struct device));
    if (!combiner) {
        return -1;
    }
    memcpy(combiner, &dev_irq_combiner, sizeof(struct device));

    vcombiner = calloc(1, sizeof(*vcombiner));
    if (!vcombiner) {
        assert(!"Unable to malloc memory for VGIC");
        return -1;
    }

    addr = create_allocated_reservation_frame(vm, IRQ_COMBINER_PADDR, seL4_CanRead, vcombiner_fault, (void *)combiner);
    assert(addr);
    if (addr == NULL) {
        return -1;
    }
    memset(addr, 0, 0x1000);
    vcombiner->vregs = (struct irq_combiner_map *)addr;
    combiner->priv = (void *)vcombiner;
    return 0;
}
