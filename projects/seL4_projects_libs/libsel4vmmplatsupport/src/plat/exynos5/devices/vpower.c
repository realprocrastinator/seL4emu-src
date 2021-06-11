/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>

#include <sel4vmmplatsupport/plat/vpower.h>
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/plat/devices.h>

#define PWR_SWRST_BANK    0
#define PWR_SWRST_OFFSET  0x400

#define PWR_SHUTDOWN_BANK 3
#define PWR_SHUTDOWN_OFFSET 0x30C

struct power_priv {
    vm_t *vm;
    vm_power_cb shutdown_cb;
    void *shutdown_token;
    vm_power_cb reboot_cb;
    void *reboot_token;
    void *regs[5];
};


static memory_fault_result_t handle_vpower_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                                 void *cookie)
{
    struct device *dev = (struct device *)cookie;
    struct power_priv *power_data = (struct power_priv *)dev->priv;
    volatile uint32_t *reg;
    int vm_offset, offset, reg_offset;
    int bank;

    /* Gather fault information */
    vm_offset = fault_addr - dev->pstart;
    bank = vm_offset >> 12;
    offset = vm_offset & MASK(12);
    reg_offset = offset & ~MASK(2);

    /* Handle the fault */
    reg = (volatile uint32_t *)(power_data->regs[bank] + offset);
    if (is_vcpu_read_fault(vcpu)) {
        set_vcpu_fault_data(vcpu, *reg);
        ZF_LOGD("[%s] pc0x%x| r0x%x:0x%x\n", dev->name, get_vcpu_fault_ip(vcpu),
                fault_addr, get_vcpu_fault_data(vcpu));
    } else {
        if (bank == PWR_SWRST_BANK && reg_offset == PWR_SWRST_OFFSET) {
            if (get_vcpu_fault_data(vcpu)) {
                /* Software reset */
                ZF_LOGD("[%s] Software reset\n", dev->name);
                if (power_data->reboot_cb) {
                    int err;
                    err = power_data->reboot_cb(vm, power_data->reboot_token);
                    if (err) {
                        return FAULT_ERROR;
                    }
                }
            }
        } else if (bank == PWR_SHUTDOWN_BANK && reg_offset == PWR_SHUTDOWN_OFFSET) {
            uint32_t new_reg = emulate_vcpu_fault(vcpu, *reg);
            new_reg &= BIT(31) | BIT(9) | BIT(8);
            if (new_reg == (BIT(31) | BIT(9))) {
                /* Software power down */
                ZF_LOGD("[%s] Power down\n", dev->name);
                if (power_data->shutdown_cb) {
                    int err;
                    err = power_data->shutdown_cb(vm, power_data->reboot_token);
                    if (err) {
                        return FAULT_ERROR;
                    }
                }
            } else {
                *reg = emulate_vcpu_fault(vcpu, *reg);
            }

        } else {
            ZF_LOGD("[%s] pc 0x%x| access violation writing 0x%x to 0x%x\n",
                    dev->name, get_vcpu_fault_ip(vcpu), get_vcpu_fault_data(vcpu),
                    fault_addr);
        }
    }
    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}

const struct device dev_alive = {
    .name = "alive",
    .pstart = ALIVE_PADDR,
    .size = 0x5000,
    .priv = NULL
};

int vm_install_vpower(vm_t *vm, vm_power_cb shutdown_cb, void *shutdown_token,
                      vm_power_cb reboot_cb, void *reboot_token)
{
    struct power_priv *power_data;
    struct device *d;
    int err;
    int i;

    d = calloc(1, sizeof(struct device));
    memcpy(d, &dev_alive, sizeof(struct device));

    /* Initialise the virtual device */
    power_data = calloc(1, sizeof(struct power_priv));
    if (power_data == NULL) {
        assert(power_data);
        free(d);
        return -1;
    }
    power_data->vm = vm;
    power_data->shutdown_cb = shutdown_cb;
    power_data->shutdown_token = shutdown_token;
    power_data->reboot_cb = reboot_cb;
    power_data->reboot_token = reboot_token;

    for (i = 0; i < d->size >> 12; i++) {
        power_data->regs[i] = ps_io_map(&vm->io_ops->io_mapper, d->pstart + (i << 12), PAGE_SIZE_4K, 0, PS_MEM_NORMAL);
        if (power_data->regs[i] == NULL) {
            free(d);
            free(power_data);
            return -1;
        }
    }

    d->priv = power_data;
    vm_memory_reservation_t *reservation = vm_reserve_memory_at(vm, d->pstart, d->size,
                                                                handle_vpower_fault, (void *)d);
    if (!reservation) {
        free(d);
        free(power_data);
        return -1;
    }
    return 0;
}
