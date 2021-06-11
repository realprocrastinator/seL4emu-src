/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4vmmplatsupport/drivers/virtio_pci_emul.h>
#include <stdbool.h>

#include "virtio_emul_helpers.h"

#define VUART_BUFLEN 4088

char buf[VUART_BUFLEN];

typedef struct console_virtio_emul_internal {
    struct console_passthrough driver;
} console_internal_t;

static void emul_con_rx_complete(void *iface, char *buf, unsigned int len)
{
    virtio_emul_t *emul = (virtio_emul_t *)iface;
    console_internal_t *con = emul->internal;
    vqueue_t *virtq = &emul->virtq;
    int i;
    struct vring *vring = &virtq->vring[RX_QUEUE];

    uint16_t guest_idx = ring_avail_idx(emul, vring);
    uint16_t idx = virtq->last_idx[RX_QUEUE];
    if (idx != guest_idx) {
        /* total length of the written packet so far */
        size_t tot_written = 0;
        /* amount of the current descriptor written */
        size_t desc_written = 0;
        /* how much we have written of the current buffer */
        size_t buf_written = 0;
        /* the current buffer. -1 indicates the virtio con buffer */
        int current_buf = 0;
        uint16_t desc_head = ring_avail(emul, vring, idx);
        /* start walking the descriptors */
        struct vring_desc desc;
        uint16_t desc_idx = desc_head;
        do {
            desc = ring_desc(emul, vring, desc_idx);
            /* determine how much we can copy */
            uint32_t copy;
            copy = len - buf_written;
            copy = MIN(copy, desc.len - desc_written);
            vm_guest_write_mem(emul->vm, buf + buf_written, (uintptr_t)desc.addr + desc_written, copy);
            /* update amounts */
            tot_written += copy;
            desc_written += copy;
            buf_written += copy;
            /* see what's gone over */
            if (desc_written == desc.len) {
                if (!desc.flags & VRING_DESC_F_NEXT) {
                    /* descriptor chain is too short to hold the whole packet.
                     * just truncate */
                    break;
                }
                desc_idx = desc.next;
                desc_written = 0;
            }
            if (buf_written == len) {
                current_buf++;
                buf_written = 0;
            }

        } while (current_buf < 1);
        /* now put it in the used ring */
        struct vring_used_elem used_elem = {desc_head, tot_written};
        ring_used_add(emul, vring, used_elem);

        /* record that we've used this descriptor chain now */
        virtq->last_idx[RX_QUEUE]++;
        /* notify the guest that there is something in its used ring */
        con->driver.handleIRQ(con->driver.console_data);
    }
}

void virtio_console_putchar(virtio_emul_t *con, char *buf, int len)
{
    emul_con_rx_complete((void *)con, buf, len);
}

static void emul_con_notify_tx(virtio_emul_t *emul)
{
    console_internal_t *con = emul->internal;
    vqueue_t *virtq = &emul->virtq;
    struct vring *vring = &virtq->vring[TX_QUEUE];
    /* read the index */
    uint16_t guest_idx = ring_avail_idx(emul, vring);
    /* process what we can of the ring */

    uint16_t idx = virtq->last_idx[TX_QUEUE];
    while (idx != guest_idx) {

        /* read the head of the descriptor chain */
        uint16_t desc_head = ring_avail(emul, vring, idx);

        /* length of the final packet to deliver */
        uint32_t len = 0;
        /* we want to skip the initial virtio header, as this should
         * not be sent to the actual ethernet driver. This records
         * how much we have skipped so far. */
        uint32_t skipped = 0;
        /* start walking the descriptors */
        struct vring_desc desc;
        uint16_t desc_idx = desc_head;
        do {
            desc = ring_desc(emul, vring, desc_idx);

            vm_guest_read_mem(emul->vm, buf + len, (uintptr_t)desc.addr, desc.len);
            len += desc.len;
            desc_idx = desc.next;
        } while (desc.flags & VRING_DESC_F_NEXT);
        /* ship it */
        for (int i = 0; i < len; i++) {
            con->driver.putchar(buf[i]);
        }
        /* next */
        idx++;
        struct vring_used_elem used_elem = {desc_head, 0};
        ring_used_add(emul, &virtq->vring[TX_QUEUE], used_elem);
        con->driver.handleIRQ(con->driver.console_data);
    }
    /* update which parts of the ring we have processed */
    virtq->last_idx[TX_QUEUE] = idx;
}

// NOTE: Both in/out are the same. Leaving stubs here incase additional features are needed.
bool console_device_emul_io_in(struct virtio_emul *emul, unsigned int offset, unsigned int size, unsigned int *result)
{
    bool handled = false;
    switch (offset) {
    case VIRTIO_PCI_HOST_FEATURES:
        handled = true;
        assert(size == 4);
        break;
    }
    return handled;
}

bool console_device_emul_io_out(struct virtio_emul *emul, unsigned int offset, unsigned int size, unsigned int value)
{
    bool handled = false;
    switch (offset) {
    case VIRTIO_PCI_GUEST_FEATURES:
        handled = true;
        assert(size == 4);
        break;
    }
    return handled;
}

void *console_virtio_emul_init(virtio_emul_t *emul, ps_io_ops_t io_ops, console_driver_init driver, void *config)
{
    console_internal_t *internal = NULL;
    int err;
    internal = calloc(1, sizeof(*internal));
    if (!internal) {
        goto error;
    }
    err = driver(&internal->driver, io_ops, config);
    emul->device_io_in = console_device_emul_io_in;
    emul->device_io_out = console_device_emul_io_out;
    if (err) {
        ZF_LOGE("Failed to initialize driver");
        goto error;
    }
    emul->notify = emul_con_notify_tx;
    return (void *)internal;
error:
    if (emul) {
        free(emul);
    }
    if (internal) {
        free(internal);
    }
    return NULL;
}
