/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

#include <sel4utils/irq_server.h>
#include <sel4vmmplatsupport/device.h>

struct combiner_irq {
    int group;
    int index;
    void *combiner_priv;
    void *priv;
};

typedef void (*combiner_irq_handler_fn)(struct combiner_irq *irq);

extern const struct device dev_irq_combiner;

/**
 * Register for an IRQ combiner IRQ
 * @param[in] group   The IRQ group (IRQ number - 32)
 * @param[in[ index   The irq index withing this group
 * @param[in] vb      A callback function to call when this IRQ fires
 * @param[in] priv    private data to pass to cb
 * @param[in] r       resources for allocation
 * @return            0 on succes
 */
int vmm_register_combiner_irq(int group, int index, combiner_irq_handler_fn cb, void *priv);

/**
 * Call to acknlowledge an IRQ with the combiner
 * @param[in] irq a description of the IRQ to ACK
 */
void combiner_irq_ack(struct combiner_irq *irq);
