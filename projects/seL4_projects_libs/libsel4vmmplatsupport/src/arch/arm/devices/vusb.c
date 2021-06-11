/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vcpu_fault.h>

#ifdef CONFIG_LIB_USB
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/arch/vusb.h>
#include <sel4vmmplatsupport/arch/service.h>
#include <sel4vmmplatsupport/guest_memory_util.h>

#include <vka/capops.h>
#include <vka/object.h>

#include <vspace/vspace.h>
#include <simple/simple.h>
#include <platsupport/io.h>
#include <sel4utils/thread.h>
#include <stdlib.h>
#include <usb/usb_host.h>
#include <usb/usb.h>

#include <string.h>

#define MAX_ACTIVE_URB   (0x1000 / sizeof(struct sel4urb))

#define SURBT_PARAM_GET_TYPE(param) (((param) >> 30) & 0x3)
#define SURBT_PARAM_GET_SIZE(param) (((param) >>  0) & 0x0fffffff)
#define SURBT_PARAM_TYPE(type)      ((type) << 30)
#define SURBT_PARAM_SIZE(size)      ((size) << 0)

#define SURB_EPADDR_STATE_INVALID   (0 << 28)
#define SURB_EPADDR_STATE_PENDING   (1 << 28)
#define SURB_EPADDR_STATE_ACTIVE    (2 << 28)
#define SURB_EPADDR_STATE_COMPLETE  (4 << 28)
#define SURB_EPADDR_STATE_SUCCESS   (SURB_EPADDR_STATE_COMPLETE | (0 << 28))
#define SURB_EPADDR_STATE_ERROR     (SURB_EPADDR_STATE_COMPLETE | (1 << 28))
#define SURB_EPADDR_STATE_CANCELLED (SURB_EPADDR_STATE_COMPLETE | (2 << 28))
#define SURB_EPADDR_STATE_MASK      (SURB_EPADDR_STATE_COMPLETE | (3 << 28))
#define SURB_EPADDR_GET_STATE(x)    ((x) & SURB_EPADDR_STATE_MASK)
#define SURB_EPADDR_GET_ADDR(x)     (((x) >>  0) & 0x7f)
#define SURB_EPADDR_GET_HUB_ADDR(x) (((x) >>  7) & 0x7f)
#define SURB_EPADDR_GET_HUB_PORT(x) (((x) >> 14) & 0x7f)
#define SURB_EPADDR_GET_EP(x)       (((x) >> 21) & 0x0f)
#define SURB_EPADDR_GET_SPEED(x)    (((x) >> 25) & 0x03)
#define SURB_EPADDR_GET_DT(x)       (((x) >> 27) & 0x01)
#define SURB_EPADDR_ADDR(x)         ((x) <<  0)
#define SURB_EPADDR_HUB_ADDR(x)     ((x) <<  7)
#define SURB_EPADDR_HUB_PORT(x)     ((x) << 14)
#define SURB_EPADDR_EP(x)           ((x) << 21)
#define SURB_EPADDR_SPEED(x)        ((x) << 25)
#define SURB_EPADDR_DT              (1 << 27)

struct sel4urbt {
    uintptr_t paddr;
    uint32_t param;
} __attribute__((packed));

struct sel4urb {
    uint32_t epaddr;
    uint16_t max_pkt;
    uint16_t rate_ms;
    uint16_t urb_bytes_remaining;
    uint16_t nxact;
    struct sel4urbt desc[2];
} __attribute__((packed));

typedef struct usb_data_regs {
    struct sel4urb sel4urb[MAX_ACTIVE_URB];
} usb_data_regs_t;

typedef struct usb_ctrl_regs {
    uint32_t status;
    uint32_t req_reply;
    uint32_t intr;
    uint32_t notify;
    uint32_t cancel_transaction;
    uint32_t nPorts;
    struct usbreq req;
} usb_ctrl_regs_t;

struct vframe {
    vka_object_t obj;
    cspacepath_t vm_cap_path, vmm_cap_path;
    void *base;
};

struct vusb_device {
    vm_t *vm;
    virq_handle_t virq;
    usb_host_t *hcd;
    struct xact int_xact;
    uint32_t rh_port_status;
    usb_data_regs_t *data_regs;
    usb_ctrl_regs_t *ctrl_regs;
    struct xact_token {
        struct vusb_device *vusb;
        int idx;
    } token[MAX_ACTIVE_URB];
    int int_pending;
};


static inline vusb_device_t *device_to_vusb_dev_data(struct device *d)
{
    return (vusb_device_t *)d->priv;
}

static inline void surb_epaddr_change_state(struct sel4urb *u, uint32_t s)
{
    uint32_t e;
    e = u->epaddr;
    e &= ~SURB_EPADDR_STATE_MASK;
    e |= s;
    u->epaddr = e;
}

static void vusb_inject_irq(vusb_device_t *vusb)
{
    vusb->ctrl_regs->intr = 1;
    if (vusb->int_pending == 0) {
        vusb->int_pending = 1;
        vm_inject_IRQ(vusb->virq);
    }
}

static int desc_to_xact(vm_t *vm, struct sel4urbt *desc, struct xact *xact)
{
    switch (SURBT_PARAM_GET_TYPE(desc->param)) {
    case 0:
        xact->type = PID_IN;
        break;
    case 1:
        xact->type = PID_OUT;
        break;
    case 2:
        xact->type = PID_SETUP;
        break;
    default:
        return -1;
    }
    xact->len = SURBT_PARAM_GET_SIZE(desc->param);
    xact->paddr = vm_arm_ipa_to_pa(vm, desc->paddr, xact->len);
    xact->vaddr = (void *)0xdeadbeef;
    return xact->paddr == 0;
}

static int sel4urb_to_xact(vm_t *vm, struct sel4urb *surb, struct xact *xact)
{
    int nxact;
    int err;
    int i;
    nxact = surb->nxact;
    assert(nxact <= 2);
    assert(nxact > 0);
    assert(SURB_EPADDR_GET_STATE(surb->epaddr) == SURB_EPADDR_STATE_PENDING);
    /* Fill translate surb to xact */
    for (i = 0; i < nxact; i++) {
        err = desc_to_xact(vm, &surb->desc[i], &xact[i]);
        if (err) {
            printf("Invalid USB descriptor\n");
            return -1;
        }
    }
    /* Check if this should be an INT packet */
    if (surb->rate_ms) {
        xact[0].type = PID_IN;
        assert(nxact == 1);
    } else if (xact[0].type == PID_SETUP) {
        if (nxact == 1) {
            /* Read in ACK */
            xact[nxact].type = PID_IN;
            xact[nxact].len = 0;
            nxact++;
        } else if (xact[nxact - 1].type == PID_IN) {
            xact[nxact].type = PID_OUT;
            xact[nxact].len = 0;
            nxact++;
        } else if (xact[nxact - 1].type == PID_OUT) {
            xact[nxact].type = PID_IN;
            xact[nxact].len = 0;
            nxact++;
        } else {
            assert(0);
        }
    } else if (xact[0].type == PID_OUT && (xact[0].len % surb->max_pkt == 0)) {
        xact[nxact].type = PID_OUT;
        xact[nxact].len = 0;
        nxact++;
    }
    return nxact;
}

static int ctrl_to_xact(usb_ctrl_regs_t *ctrl, struct xact *xact)
{
    xact[0].len = sizeof(struct usbreq);
    xact[0].vaddr = (void *)&ctrl->req;
    xact[0].paddr = 0xdeadbeef;
    xact[0].type = PID_SETUP;
    xact[1].len = ctrl->req.wLength;
    xact[1].vaddr = (void *)&ctrl->req_reply;
    xact[1].paddr = 0xdeadbeef;
    if (ctrl->req.bmRequestType & BIT(7)) {
        /* Device to host */
        xact[1].type = PID_IN;
    } else {
        /* Host to device */
        xact[1].type = PID_OUT;
    }
    return 0;
}

static int root_hub_ctrl_start(usb_host_t *hcd, usb_ctrl_regs_t *ctrl)
{
    struct xact xact[2];
    struct endpoint ep;
    int len;
    int err;
    err = ctrl_to_xact(ctrl, xact);
    if (err) {
        return -1;
    }
    ep.type = EP_CONTROL;
    ep.num = 0;
    ep.max_pkt = 64;
    len = usb_hcd_schedule(hcd, 1, -1, 0, 0, &ep, xact, 2, NULL, NULL);
    ZF_LOGD("usb ctrl complete len %d\n", len);
    return len;
}

static void vm_vusb_cancel(vusb_device_t *vusb, uint32_t surb_idx)
{
    if (surb_idx < MAX_ACTIVE_URB) {
        usb_hcd_cancel(vusb->hcd, &vusb->token[surb_idx]);
    }
}

static memory_fault_result_t handle_vusb_fault(vm_t *vm, vm_vcpu_t *vcpu, uintptr_t fault_addr, size_t fault_length,
                                               void *cookie)
{
    vusb_device_t *vusb;
    usb_ctrl_regs_t *ctrl_regs;
    uint32_t *reg;
    int offset;
    struct device *d = (struct device *)cookie;

    assert(d);
    assert(d->priv);
    offset = fault_addr - d->pstart - 0x1000;
    vusb = device_to_vusb_dev_data(d);
    ctrl_regs = vusb->ctrl_regs;
    reg = (uint32_t *)((void *)ctrl_regs + (offset & ~0x3));
    if (is_vcpu_read_fault(vcpu)) {
        if (reg != &ctrl_regs->status) {
            set_vcpu_fault_data(vcpu, *reg);
        }
    } else {
        if (reg == &ctrl_regs->status) {
            /* start a transfer */
            root_hub_ctrl_start(vusb->hcd, ctrl_regs);
        } else if (reg == &ctrl_regs->intr) {
            /* Clear the interrupt pending flag */
            *reg = emulate_vcpu_fault(vcpu, *reg);
        } else if (reg == &ctrl_regs->notify) {
            /* Manual notification */
            vm_vusb_notify(vusb);
        } else if (reg == &ctrl_regs->cancel_transaction) {
            /* Manual notification */
            vm_vusb_cancel(vusb, get_vcpu_fault_data(vcpu));
        } else if ((void *)reg >= (void *)&ctrl_regs->req) {
            /* Fill out the root hub USB request */
            *reg = emulate_vcpu_fault(vcpu, *reg);
        }
    }
    advance_vcpu_fault(vcpu);
    return FAULT_HANDLED;
}

const struct device dev_vusb = {
    .name = "virtual usb",
    .pstart = 0xDEADBEEF,
    .size = 0x2000,
    .priv = NULL
};

/* Called by the VM to ACK a virtual IRQ */
static void vusb_ack(void *token)
{
    vusb_device_t *vusb = token;
    if (vusb->ctrl_regs->intr) {
        /* Another IRQ occured */
        vm_inject_IRQ(vusb->virq);
    } else {
        vusb->int_pending = 0;
    }
}

/* Callback for root hub status changes */
static int usb_sts_change(void *token, enum usb_xact_status stat, int rbytes)
{
    vusb_device_t *vusb = (vusb_device_t *)token;
    vusb->ctrl_regs->status = *(uint32_t *)vusb->int_xact.vaddr;
    return 1;
}

static int vusb_complete_cb(void *token, enum usb_xact_status stat, int rbytes)
{
    struct xact_token *t = (struct xact_token *)token;
    vusb_device_t *vusb = t->vusb;
    int surb_idx = t->idx;
    struct sel4urb *surb = &vusb->data_regs->sel4urb[surb_idx];
    uint32_t status;

    ZF_LOGD("packet completion callback %d\n", surb_idx);
    surb->urb_bytes_remaining = rbytes;
    switch (stat) {
    case XACTSTAT_SUCCESS:
        status = SURB_EPADDR_STATE_COMPLETE;
        break;
    case XACTSTAT_CANCELLED:
        status = SURB_EPADDR_STATE_CANCELLED;
        break;
    case XACTSTAT_ERROR:
        status = SURB_EPADDR_STATE_ERROR;
        break;
    case XACTSTAT_PENDING:
    case XACTSTAT_HOSTERROR:
    default:
        status = SURB_EPADDR_STATE_ERROR;
    }
    surb_epaddr_change_state(surb, status);
    vusb_inject_irq(vusb);

    return 0;
}

void vm_vusb_notify(vusb_device_t *vusb)
{
    struct sel4urb *u;
    struct xact xact[3];
    enum usb_speed speed;
    struct endpoint ep;
    int len;
    int nxact;
    int i;
    for (i = 0; i < MAX_ACTIVE_URB; i++) {
        struct xact_token *t;
        u = &vusb->data_regs->sel4urb[i];
        if (SURB_EPADDR_GET_STATE(u->epaddr) != SURB_EPADDR_STATE_PENDING) {
            continue;
        }
        ZF_LOGD("descriptor %d ACTIVE\n", i);

        switch (SURB_EPADDR_GET_SPEED(u->epaddr)) {
        case 3:
            speed = USBSPEED_HIGH;
            break;
        default:
            printf("Unknown USB speed %d\n", SURB_EPADDR_GET_SPEED(u->epaddr));
            speed = USBSPEED_HIGH;
            assert(0);
        }

        nxact = sel4urb_to_xact(vusb->vm, u, xact);
        if (nxact < 0) {
            ZF_LOGD("descriptor error\n");
            surb_epaddr_change_state(u, SURB_EPADDR_STATE_ERROR);
            vusb_inject_irq(vusb);
            return;
        }
        t = &vusb->token[i];
        t->vusb = vusb;
        t->idx = i;
        ep.num = SURB_EPADDR_EP(u->epaddr);
        ep.max_pkt = u->max_pkt;
        ep.interval = u->rate_ms;
        len = usb_hcd_schedule(vusb->hcd, SURB_EPADDR_GET_ADDR(u->epaddr),
                               SURB_EPADDR_GET_HUB_ADDR(u->epaddr),
                               SURB_EPADDR_GET_HUB_PORT(u->epaddr),
                               speed, &ep,
                               xact, nxact, &vusb_complete_cb, t);
        if (len < 0) {
            surb_epaddr_change_state(u, SURB_EPADDR_STATE_ERROR);
            vusb_inject_irq(vusb);
        } else {
            surb_epaddr_change_state(u, SURB_EPADDR_STATE_ACTIVE);
        }
    }
}

vusb_device_t *vm_install_vusb(vm_t *vm, usb_host_t *hcd, uintptr_t pbase, int virq,
                               seL4_CPtr vmm_ncap, seL4_CPtr vm_ncap, int badge)
{
    vusb_device_t *vusb;
    struct device *d;
    struct endpoint ep;
    int err;

    d = (struct device *)calloc(1, sizeof(struct device));
    if (!d) {
        return NULL;
    }

    /* Setup book keeping */
    vusb = calloc(1, sizeof(*vusb));
    if (vusb == NULL) {
        return NULL;
    }
    vusb->vm = vm;
    vusb->hcd = hcd;

    /* Setup the device */
    d->pstart = pbase;
    d->size = 0x2000;
    d->priv = vusb;

    /* Map registers */
    vusb->data_regs = create_allocated_reservation_frame(vm, pbase, seL4_AllRights, default_error_fault_callback, NULL);
    if (vusb->data_regs == NULL) {
        free(vusb);
        return NULL;
    }
    vusb->ctrl_regs = create_allocated_reservation_frame(vm, pbase + 0x1000, seL4_CanRead, handle_vusb_fault, (void *)d);
    if (vusb->ctrl_regs == NULL) {
        free(vusb);
        return NULL;
    }
    /* Initialise virtual registers */
    vusb->ctrl_regs->nPorts = usb_hcd_count_ports(hcd);
    assert(vusb->ctrl_regs->nPorts);
    vusb->ctrl_regs->req_reply = 0;
    vusb->ctrl_regs->status = 0;
    vusb->ctrl_regs->intr = 0;
    vusb->int_pending = 0;

    /* Initialise virtual IRQ */
    vusb->virq = vm_virq_new(vm, virq, &vusb_ack, vusb);
    if (vusb->virq == NULL) {
        return NULL;
    }

    err = vmm_install_service(vm, vmm_ncap, vm_ncap, badge);
    if (err) {
        assert(!err);
        return NULL;
    }

    /* Schedule Root hub INT packet */
    vusb->int_xact.type = PID_IN;
    vusb->int_xact.len = (vusb->ctrl_regs->nPorts + 7) / 8;
    vusb->int_xact.paddr = 0xdeadbeef;
    vusb->int_xact.vaddr = (void *)&vusb->rh_port_status;
    ep.num = 1;
    ep.max_pkt = 2;
    ep.interval = 10;
    err = usb_hcd_schedule(vusb->hcd, 1, -1, 0, 0, &ep,
                           &vusb->int_xact, 1, usb_sts_change, vusb);
    if (err) {
        assert(!err);
        return NULL;
    } else {
        return vusb;
    }
}

#endif /* CONFIG_LIB_USB */
