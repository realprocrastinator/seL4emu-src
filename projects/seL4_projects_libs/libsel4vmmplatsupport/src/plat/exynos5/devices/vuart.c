/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>
#include <sel4vm/guest_memory.h>

#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/plat/device_map.h>
#include <sel4vmmplatsupport/plat/vuart.h>
#include <sel4vmmplatsupport/plat/devices.h>

#define VUART_BUFLEN 300

#define ULCON       0x000 /* line control */
#define UCON        0x004 /* control */
#define UFCON       0x008 /* fifo control */
#define UMCON       0x00C /* modem control */
#define UTRSTAT     0x010 /* TX/RX status */
#define UERSTAT     0x014 /* RX error status */
#define UFSTAT      0x018 /* FIFO status */
#define UMSTAT      0x01C /* modem status */
#define UTXH        0x020 /* TX buffer */
#define URXH        0x024 /* RX buffer */
#define UBRDIV      0x028 /* baud rate divisor */
#define UFRACVAL    0x02C /* divisor fractional value */
#define UINTP       0x030 /* interrupt pending */
#define UINTSP      0x034 /* interrupt source pending */
#define UINTM       0x038 /* interrupt mask */
#define UART_SIZE   0x03C



struct vuart_priv {
    void *regs;
    char buffer[VUART_BUFLEN];
    int buf_pos;
    vm_t *vm;
};

static inline void *vuart_priv_get_regs(struct device *d)
{
    return ((struct vuart_priv *)d->priv)->regs;
}

static void vuart_reset(struct device *d)
{
    const uint32_t reset_data[] = {
        0x00000003, 0x000003c5, 0x00000111, 0x00000000,
        0x00000002, 0x00000000, 0x00010000, 0x00000011,
        0x00000000, 0x00000000, 0x00000021, 0x0000000b,
        0x00000004, 0x00000004, 0x00000000
    };
    assert(sizeof(reset_data) == UART_SIZE);
    memcpy(vuart_priv_get_regs(d), reset_data, sizeof(reset_data));
}

static void flush_vconsole_device(struct device *d)
{
    struct vuart_priv *vuart_data;
    char *buf;
    int i;
    assert(d->priv);
    vuart_data = (struct vuart_priv *)d->priv;
    buf = vuart_data->buffer;
    for (i = 0; i < vuart_data->buf_pos; i++) {
        if (buf[i] != '\033') {
            putchar(buf[i]);
        } else {
            while (i < vuart_data->buf_pos && buf[i] != 'm') {
                i++;
            }
        }
    }
    vuart_data->buf_pos = 0;
}

static void vuart_putchar(struct device *d, char c)
{
    struct vuart_priv *vuart_data;
    assert(d->priv);
    vuart_data = (struct vuart_priv *)d->priv;

    if (vuart_data->buf_pos == VUART_BUFLEN) {
        flush_vconsole_device(d);
    }
    assert(vuart_data->buf_pos < VUART_BUFLEN);
    vuart_data->buffer[vuart_data->buf_pos++] = c;

    if ((c & 0xff) == '\n') {
        flush_vconsole_device(d);
    }
}

static memory_fault_result_t handle_vuart_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                                void *cookie)
{
    uint32_t *reg;
    int offset;
    uint32_t mask;
    struct device *dev;
    dev = (struct device *)cookie;

    /* Gather fault information */
    offset = fault_addr - dev->pstart;
    reg = (uint32_t *)(vuart_priv_get_regs(dev) + offset);
    mask = get_vcpu_fault_data_mask(vcpu);
    /* Handle the fault */
    if (offset < 0 || UART_SIZE <= offset) {
        /* Out of range, treat as SBZ */
        set_vcpu_fault_data(vcpu, 0);
        return FAULT_IGNORE;

    } else if (is_vcpu_read_fault(vcpu)) {
        /* Blindly read out data */
        set_vcpu_fault_data(vcpu, *reg);
        advance_vcpu_fault(vcpu);

    } else { /* if(fault_is_write(fault))*/
        /* Blindly write to the device */
        uint32_t v;
        v = *reg & ~mask;
        v |= get_vcpu_fault_data(vcpu) & mask;
        *reg = v;
        /* If it was the TX buffer, we send to the local stdout */
        if (offset == UTXH) {
            vuart_putchar(dev, get_vcpu_fault_data(vcpu));
        }
        advance_vcpu_fault(vcpu);
    }
    return FAULT_HANDLED;
}

const struct device dev_uart0 = {
    .name = "uart0",
    .pstart = UART0_PADDR,
    .size = 0x1000,
    .priv = NULL
};

const struct device dev_uart1 = {
    .name = "uart1",
    .pstart = UART1_PADDR,
    .size = 0x1000,
    .priv = NULL
};

const struct device dev_uart2 = {
    .name = "uart2.console",
    .pstart = UART2_PADDR,
    .size = 0x1000,
    .priv = NULL
};



const struct device dev_uart3 = {
    .name = "uart3",
    .pstart = UART3_PADDR,
    .size = 0x1000,
    .priv = NULL
};


int vm_install_vconsole(vm_t *vm)
{
    struct vuart_priv *vuart_data;
    struct device *d;
    int err;

    d = (struct device *)calloc(1, sizeof(struct device));
    if (!d) {
        return -1;
    }

    *d = dev_vconsole;
    /* Initialise the virtual device */
    vuart_data = calloc(1, sizeof(struct vuart_priv));
    if (vuart_data == NULL) {
        assert(vuart_data);
        return -1;
    }
    vuart_data->vm = vm;

    vuart_data->regs = calloc(1, UART_SIZE);
    if (vuart_data->regs == NULL) {
        assert(vuart_data->regs);
        return -1;
    }

    vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, d->pstart, d->size,
                                                                handle_vuart_fault, (void *)d);
    if (!reservation) {
        return -1;
    }
    d->priv = vuart_data;
    vuart_reset(d);
    return 0;
}


int vm_install_ac_uart(vm_t *vm, const struct device *d)
{
    int err;
    int mask_size = UART_SIZE;
    uint32_t *mask = (uint32_t *)calloc(1, mask_size);
    err = vm_install_generic_ac_device(vm, d, mask, mask_size, VACDEV_MASK_ONLY);
    if (err) {
        free(mask);
        return -1;
    } else {
        mask[ULCON    / 4] = 0x0;
        mask[UCON     / 4] = 0x0;
        mask[UFCON    / 4] = 0x0;
        mask[UMCON    / 4] = 0x0;
        mask[UTRSTAT  / 4] = 0x0;
        mask[UERSTAT  / 4] = 0x0;
        mask[UFSTAT   / 4] = 0x0;
        mask[UMSTAT   / 4] = 0x0;
        mask[UTXH     / 4] = 0xffffffff;
        mask[URXH     / 4] = 0xffffffff;
        mask[UBRDIV   / 4] = 0x0;
        mask[UFRACVAL / 4] = 0x0;
        mask[UINTP    / 4] = 0xffffffff;
        mask[UINTSP   / 4] = 0xffffffff;
        mask[UINTM    / 4] = 0xffffffff;
        return 0;
    }
}
