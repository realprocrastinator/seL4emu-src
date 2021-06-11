/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdio.h>
#include <assert.h>
#include <utils/util.h>

#include "fault.h"

static seL4_Word wzr = 0;
seL4_Word *decode_rt(int reg, seL4_UserContext *c)
{
    switch (reg) {
    case  0:
        return &c->x0;
    case  1:
        return &c->x1;
    case  2:
        return &c->x2;
    case  3:
        return &c->x3;
    case  4:
        return &c->x4;
    case  5:
        return &c->x5;
    case  6:
        return &c->x6;
    case  7:
        return &c->x7;
    case  8:
        return &c->x8;
    case  9:
        return &c->x9;
    case 10:
        return &c->x10;
    case 11:
        return &c->x11;
    case 12:
        return &c->x12;
    case 13:
        return &c->x13;
    case 14:
        return &c->x14;
    case 15:
        return &c->x15;
    case 16:
        return &c->x16;
    case 17:
        return &c->x17;
    case 18:
        return &c->x18;
    case 19:
        return &c->x19;
    case 20:
        return &c->x20;
    case 21:
        return &c->x21;
    case 22:
        return &c->x22;
    case 23:
        return &c->x23;
    case 24:
        return &c->x24;
    case 25:
        return &c->x25;
    case 26:
        return &c->x26;
    case 27:
        return &c->x27;
    case 28:
        return &c->x28;
    case 29:
        return &c->x29;
    case 30:
        return &c->x30;
    case 31:
        return &wzr;
    default:
        printf("invalid reg %d\n", reg);
        assert(!"Invalid register");
        return NULL;
    }
};

#define PREG(regs, r)    printf(#r ": 0x%lx\n", regs->r)
void print_ctx_regs(seL4_UserContext *regs)
{
    PREG(regs, x0);
    PREG(regs, x1);
    PREG(regs, x2);
    PREG(regs, x3);
    PREG(regs, x4);
    PREG(regs, x5);
    PREG(regs, x6);
    PREG(regs, x7);
    PREG(regs, x8);
    PREG(regs, x9);
    PREG(regs, x10);
    PREG(regs, x11);
    PREG(regs, x12);
    PREG(regs, pc);
    PREG(regs, x14);
    PREG(regs, sp);
    PREG(regs, spsr);

    PREG(regs, x13);
    PREG(regs, x15);
    PREG(regs, x16);
    PREG(regs, x17);
    PREG(regs, x18);
    PREG(regs, x19);
    PREG(regs, x20);
    PREG(regs, x21);

    PREG(regs, x22);
    PREG(regs, x23);
    PREG(regs, x24);
    PREG(regs, x25);
    PREG(regs, x26);
    PREG(regs, x27);
    PREG(regs, x28);
    PREG(regs, x29);
    PREG(regs, x30);
}

int decode_vcpu_reg(int rt, fault_t *f)
{
    return seL4_VCPUReg_Num;
}

void fault_print_data(fault_t *fault)
{
    seL4_Word data;
    data = fault_get_data(fault) & fault_get_data_mask(fault);
    switch (fault_get_width(fault)) {
    case WIDTH_WORD:
        printf("0x%16lx", data);
        break;
    case WIDTH_HALFWORD:
        printf("0x%8lx", data);
        break;
    case WIDTH_BYTE:
        printf("0x%02lx", data);
        break;
    default:
        printf("<Invalid width> 0x%lx", data);
    }
}

bool fault_is_thumb(fault_t *f)
{
    return CPSR_IS_THUMB(fault_get_ctx(f)->spsr);
}
