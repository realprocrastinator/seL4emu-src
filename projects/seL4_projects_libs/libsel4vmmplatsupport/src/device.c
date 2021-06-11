/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/device.h>

int device_list_init(device_list_t *list)
{
    list->num_devices = 0;
    list->devices = NULL;
    return 0;
}

static int dev_cmp(const void *a, const void *b)
{
    return ((const struct device *)a)->pstart - ((const struct device *)b)->pstart;
}

int add_device(device_list_t *list, const struct device *d)
{
    if (!list || !d) {
        return -1;
    }

    struct device *updated_devices = realloc(list->devices, sizeof(struct device) * (list->num_devices + 1));
    if (!updated_devices) {
        return -1;
    }
    list->devices = updated_devices;
    memcpy(&list->devices[list->num_devices++], d, sizeof(struct device));
    qsort(list->devices, list->num_devices, sizeof(struct device), dev_cmp);
    return 0;
}

struct device *
find_device_by_pa(device_list_t *dev_list, uintptr_t addr)
{
    struct device *dev = NULL;
    for (int i = 0; i < dev_list->num_devices; i++) {
        struct device *curr_dev = &dev_list->devices[i];
        if (addr < curr_dev->pstart) {
            break;
        }

        if (addr < curr_dev->pstart + curr_dev->size) {
            /* Found a match */
            dev = curr_dev;
            break;
        }
    }
    return dev;
}
