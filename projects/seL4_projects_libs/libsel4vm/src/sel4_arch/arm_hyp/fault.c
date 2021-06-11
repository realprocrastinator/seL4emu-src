/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdio.h>
#include <assert.h>
#include <utils/util.h>

#include <sel4vm/sel4_arch/processor.h>

#include "fault.h"

seL4_Word *decode_rt(int reg, seL4_UserContext *c)
{
    switch (reg) {
    case  0:
        return &c->r0;
    case  1:
        return &c->r1;
    case  2:
        return &c->r2;
    case  3:
        return &c->r3;
    case  4:
        return &c->r4;
    case  5:
        return &c->r5;
    case  6:
        return &c->r6;
    case  7:
        return &c->r7;
    case  8:
        return &c->r8;
    case  9:
        return &c->r9;
    case 10:
        return &c->r10;
    case 11:
        return &c->r11;
    case 12:
        return &c->r12;
    case 13:
        return &c->sp;
    case 14:
        return &c->r14; /* lr */
    case 15:
        return &c->pc;
    default:
        assert(!"Invalid register");
        return NULL;
    }
};

#define PREG(regs, r)    printf(#r ": 0x%x\n", regs->r)
void print_ctx_regs(seL4_UserContext *regs)
{
    PREG(regs, r0);
    PREG(regs, r1);
    PREG(regs, r2);
    PREG(regs, r3);
    PREG(regs, r4);
    PREG(regs, r5);
    PREG(regs, r6);
    PREG(regs, r7);
    PREG(regs, r8);
    PREG(regs, r9);
    PREG(regs, r10);
    PREG(regs, r11);
    PREG(regs, r12);
    PREG(regs, pc);
    PREG(regs, r14);
    PREG(regs, sp);
    PREG(regs, cpsr);
}

/**
 * Returns a seL4_VCPUReg if the fault affects a banked register.  Otherwise
 * seL4_VCPUReg_Num is returned.  It uses the fault to look up what mode the
 * processor is in and based on rt returns a banked register.
 */
int decode_vcpu_reg(int rt, fault_t *f)
{
    assert(f->pmode != PMODE_HYPERVISOR);
    if (f->pmode == PMODE_USER || f->pmode == PMODE_SYSTEM) {
        return seL4_VCPUReg_Num;
    }

    int reg = seL4_VCPUReg_Num;
    if (f->pmode == PMODE_FIQ) {
        switch (rt) {
        case 8:
            reg = seL4_VCPUReg_R8fiq;
            break;
        case 9:
            reg = seL4_VCPUReg_R9fiq;
            break;
        case 10:
            reg = seL4_VCPUReg_R10fiq;
            break;
        case 11:
            reg = seL4_VCPUReg_R11fiq;
            break;
        case 12:
            reg = seL4_VCPUReg_R12fiq;
            break;
        case 13:
            reg = seL4_VCPUReg_SPfiq;
            break;
        case 14:
            reg = seL4_VCPUReg_LRfiq;
            break;
        default:
            reg = seL4_VCPUReg_Num;
            break;
        }

    } else if (rt == 13) {
        switch (f->pmode) {
        case PMODE_IRQ:
            reg = seL4_VCPUReg_SPirq;
            break;
        case PMODE_SUPERVISOR:
            reg = seL4_VCPUReg_SPsvc;
            break;
        case PMODE_ABORT:
            reg = seL4_VCPUReg_SPabt;
            break;
        case PMODE_UNDEFINED:
            reg = seL4_VCPUReg_SPund;
            break;
        default:
            ZF_LOGF("Invalid processor mode");
        }

    } else if (rt == 14) {
        switch (f->pmode) {
        case PMODE_IRQ:
            reg = seL4_VCPUReg_LRirq;
            break;
        case PMODE_SUPERVISOR:
            reg = seL4_VCPUReg_LRsvc;
            break;
        case PMODE_ABORT:
            reg = seL4_VCPUReg_LRabt;
            break;
        case PMODE_UNDEFINED:
            reg = seL4_VCPUReg_LRund;
            break;
        default:
            ZF_LOGF("Invalid processor mode");
        }

    }

    return reg;
}

void fault_print_data(fault_t *fault)
{
    seL4_Word data;
    data = fault_get_data(fault) & fault_get_data_mask(fault);
    switch (fault_get_width(fault)) {
    case WIDTH_WORD:
        printf("0x%8x", data);
        break;
    case WIDTH_HALFWORD:
        printf("0x%4x", data);
        break;
    case WIDTH_BYTE:
        printf("0x%02x", data);
        break;
    default:
        printf("<Invalid width> 0x%x", data);
    }
}

bool fault_is_thumb(fault_t *f)
{
    return CPSR_IS_THUMB(fault_get_ctx(f)->cpsr);
}
