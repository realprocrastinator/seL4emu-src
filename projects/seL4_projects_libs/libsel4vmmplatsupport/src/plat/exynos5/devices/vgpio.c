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
#include <sel4vmmplatsupport/plat/device_map.h>
#include <sel4vmmplatsupport/plat/vgpio.h>

#include <platsupport/gpio.h>
#include <platsupport/plat/gpio.h>

#define GPIOREG_CON_OFFSET    0x00
#define GPIOREG_DAT_OFFSET    0x04
#define GPIOREG_PUD_OFFSET    0x08
#define GPIOREG_DRV_OFFSET    0x0C
#define GPIOREG_CONPDN_OFFSET 0x10
#define GPIOREG_PUDPDN_OFFSET 0x14
#define GPIOREG_SIZE          0x20
#define GPIOREG_OFFSET_MASK   ((GPIOREG_SIZE - 1) & ~MASK(2))

#define GPIOREG_CON_BITS    4
#define GPIOREG_DAT_BITS    1
#define GPIOREG_PUD_BITS    2
#define GPIOREG_DRV_BITS    2
#define GPIOREG_CONPDN_BITS 2
#define GPIOREG_PUDPDN_BITS 2

#define NPORTS_PER_BANK (0x1000 / GPIOREG_SIZE)
#define NPINS_PER_PORT  (8)
#define NPINS_PER_BANK (NPORTS_PER_BANK * NPINS_PER_PORT)

static memory_fault_result_t handle_vgpio_right_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr,
                                                      size_t fault_length, void *cookie);
static memory_fault_result_t handle_vgpio_left_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr,
                                                     size_t fault_length, void *cookie);

struct gpio_device {
    vm_t *vm;
    enum vacdev_action action;
    void *regs[GPIO_NBANKS];
    uint8_t granted_bf[GPIO_NBANKS][((NPINS_PER_BANK - 1 / 8) + 1)];
};

const struct device dev_gpio_left = {
    .name = "gpio.left",
    .pstart = GPIO_LEFT_PADDR,
    .size = 0x1000,
    .priv = NULL
};

const struct device dev_gpio_right = {
    .name = "gpio.right",
    .pstart = GPIO_RIGHT_PADDR,
    .size = 0x1000,
    .priv = NULL
};

static const struct device *gpio_devices[] = {
    [GPIO_LEFT_BANK]  = &dev_gpio_left,
    [GPIO_RIGHT_BANK] = &dev_gpio_right,
    [GPIO_C2C_BANK]   = NULL,
    [GPIO_AUDIO_BANK] = NULL,
};

static uint32_t _create_mask(uint8_t ac, int bits)
{
    uint32_t mask = 0;
    while (ac) {
        int pin = CTZ(ac);
        ac &= ~BIT(pin);
        mask |= MASK(bits) << (bits * pin);
    }
    return mask;
}

static memory_fault_result_t handle_vgpio_fault(vm_t *vm, vm_vcpu_t *vcpu, struct device *dev, uintptr_t addr,
                                                size_t len,  int bank)
{
    struct gpio_device *gpio_device = (struct gpio_device *)dev->priv;
    volatile uint32_t *reg;
    int offset;
    if (gpio_device->regs[bank] == NULL) {
        /* We could not map the device. Lets return SBZ */
        if (is_vcpu_read_fault(vcpu)) {
            set_vcpu_fault_data(vcpu, 0);
        }
        advance_vcpu_fault(vcpu);
        return FAULT_HANDLED;
    }

    /* Gather fault information */
    offset = addr - dev->pstart;
    reg = (volatile uint32_t *)(gpio_device->regs[bank] + offset);
    if (is_vcpu_read_fault(vcpu)) {
        set_vcpu_fault_data(vcpu, *reg);
        ZF_LOGD("[%s] pc 0x%08x | r 0x%08x:0x%08x\n", gpio_devices[bank]->name,
                get_vcpu_fault_ip(vcpu), addr,
                get_vcpu_fault_data(vcpu));
    } else {
        uint32_t mask;
        uint32_t change;
        ZF_LOGD("[%s] pc 0x%08x | w 0x%08x:0x%08x\n", gpio_devices[bank]->name,
                get_vcpu_fault_ip(vcpu), addr,
                get_vcpu_fault_data(vcpu));
        if ((offset >= 0x700 && offset < 0xC00) || offset >= 0xE00) {
            /* Not implemented */
            mask = 0xFFFFFFFF;
        } else {
            /* GPIO and MUX general registers */
            uint32_t ac;
            int port;
            port = offset / GPIOREG_SIZE;
            assert(port < sizeof(gpio_device->granted_bf[bank]));
            ac = gpio_device->granted_bf[bank][port];
            switch (offset & GPIOREG_OFFSET_MASK) {
            case GPIOREG_CON_OFFSET:
                mask = _create_mask(ac, GPIOREG_CON_BITS);
                break;
            case GPIOREG_DAT_OFFSET:
                mask = _create_mask(ac, GPIOREG_DAT_BITS);
                break;
            case GPIOREG_PUD_OFFSET:
                mask = _create_mask(ac, GPIOREG_PUD_BITS);
                break;
            case GPIOREG_DRV_OFFSET:
                mask = _create_mask(ac, GPIOREG_DRV_BITS);
                break;
            case GPIOREG_CONPDN_OFFSET:
                mask = _create_mask(ac, GPIOREG_CONPDN_BITS);
                break;
            case GPIOREG_PUDPDN_OFFSET:
                mask = _create_mask(ac, GPIOREG_PUDPDN_BITS);
                break;
            default:  /* reserved */
                ZF_LOGE("reserved GPIO 0x%x\n", offset);
                mask = 0;
            }
        }
        change = emulate_vcpu_fault(vcpu, *reg);
        if ((change ^ *reg) & ~mask) {
            switch (gpio_device->action) {
            case VACDEV_REPORT_AND_MASK:
                change = (change & mask) | (*reg & ~mask);
            case VACDEV_REPORT_ONLY:
                /* Fallthrough */
                printf("[%s] pc 0x%08x| w @ 0x%08x: 0x%08x -> 0x%08x\n",
                       gpio_devices[bank]->name,  get_vcpu_fault_ip(vcpu),
                       addr, *reg, change);
                break;
            default:
                break;
            }
        }
        *reg = change;
    }
    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}

static memory_fault_result_t handle_vgpio_right_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr,
                                                      size_t fault_length, void *cookie)
{
    struct device *dev = (struct device *)cookie;
    return handle_vgpio_fault(vm, vcpu, dev, fault_addr, fault_length, GPIO_RIGHT_BANK);
}

static memory_fault_result_t handle_vgpio_left_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr,
                                                     size_t fault_length, void *cookie)
{
    struct device *dev = (struct device *)cookie;
    return handle_vgpio_fault(vm, vcpu, dev, fault_addr, fault_length, GPIO_LEFT_BANK);
}


struct gpio_device *
vm_install_ac_gpio(vm_t *vm, enum vacdev_default default_ac, enum vacdev_action action)
{
    struct gpio_device *gpio_device;
    vspace_t *vmm_vspace;
    uint8_t ac = (default_ac == VACDEV_DEFAULT_ALLOW) ? 0xff : 0x00;
    int i;

    gpio_device = (struct gpio_device *)calloc(1, sizeof(*gpio_device));
    if (gpio_device == NULL) {
        return NULL;
    }
    gpio_device->vm = vm;
    gpio_device->action = action;
    memset(gpio_device->granted_bf, ac, sizeof(gpio_device->granted_bf));
    for (i = 0; i < GPIO_NBANKS; i++) {
        /* Map the device locally */
        if (gpio_devices[i] != NULL) {
            struct device dev;
            int err;
            dev = *gpio_devices[i];
            dev.priv = gpio_device;
            gpio_device->regs[i] = ps_io_map(&vm->io_ops->io_mapper, dev.pstart, PAGE_SIZE_4K, 0, PS_MEM_NORMAL);
            /* Ignore failues. We will check regs for NULL on access */
            if (gpio_device->regs[i] != NULL) {
                memory_fault_callback_fn callback = (i == GPIO_LEFT_BANK) ? handle_vgpio_left_fault : handle_vgpio_right_fault;
                vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, dev.pstart, dev.size,
                                                                            callback, (void *)&dev);
            } else {
                LOG_INFO("Failed to provide device region 0x%08x->0x%08x", dev.pstart, dev.pstart + dev.size);
            }
        }
    }
    return gpio_device;
}

static int vm_gpio_config(struct gpio_device *gpio_device, gpio_id_t gpio_id, int provide)
{
    int gpioport;
    int port, pin, bank;
    gpioport = GPIOID_PORT(gpio_id);
    bank = GPIOPORT_GET_BANK(gpioport);
    port = GPIOPORT_GET_PORT(gpioport);
    pin = GPIOID_PIN(gpio_id);

    if (provide) {
        gpio_device->granted_bf[bank][port] |= BIT(pin);
    } else {
        gpio_device->granted_bf[bank][port] &= ~BIT(pin);
    }
    return 0;
}

int vm_gpio_provide(struct gpio_device *gpio_device, gpio_id_t gpio_id)
{
    return vm_gpio_config(gpio_device, gpio_id, 1);
}

int vm_gpio_restrict(struct gpio_device *gpio_device, gpio_id_t gpio_id)
{
    return vm_gpio_config(gpio_device, gpio_id, 0);
}
