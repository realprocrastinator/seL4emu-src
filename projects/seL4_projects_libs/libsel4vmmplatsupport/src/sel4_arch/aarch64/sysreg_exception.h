/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4vm/guest_vm.h>
#include <sel4vm/arch/processor.h>
#include <sel4vm/sel4_arch/processor.h>
#include <sel4vmmplatsupport/arch/guest_vcpu_fault.h>

#define SYSREG_OP0_SHIFT    20
#define SYSREG_OP0_MASK     (0b11 << SYSREG_OP0_SHIFT)
#define SYSREG_OP2_SHIFT    17
#define SYSREG_OP2_MASK     (0b111 << SYSREG_OP2_SHIFT)
#define SYSREG_OP1_SHIFT    14
#define SYSREG_OP1_MASK     (0b111 << SYSREG_OP1_SHIFT)
#define SYSREG_CRn_SHIFT    10
#define SYSREG_CRn_MASK     (0b1111 << SYSREG_CRn_SHIFT)
#define SYSREG_Rt_SHIFT     5
#define SYSREG_Rt_MASK      (0b11111 << SYSREG_Rt_SHIFT)
#define SYSREG_CRm_SHIFT    1
#define SYSREG_CRm_MASK     (0b1111 << SYSREG_CRm_SHIFT)
#define SYSREG_DIR_SHIFT    0
#define SYSREG_DIR_MASK     (0b1 << SYSREG_Rt_SHIFT)

#define SYSREG_MATCH_ALL_MASK HSR_ISS_MASK

typedef union sysreg {
    uint32_t hsr_val;
    struct sysreg_params {
        uint32_t direction: 1;
        uint32_t crm: 4;
        uint32_t rt: 5;
        uint32_t crn: 4;
        uint32_t op1: 3;
        uint32_t op2: 3;
        uint32_t op0: 2;
        uint32_t res0: 3;
        uint32_t instr_len: 1;
        uint32_t ec: 6;
    } params;
} sysreg_t;

typedef int (*sysreg_exception_handler_fn)(vm_vcpu_t *vcpu, sysreg_t *sysreg_reg, bool is_read);

typedef struct sysreg_entry {
    sysreg_t sysreg;
    sysreg_t sysreg_match_mask;
    sysreg_exception_handler_fn handler;
} sysreg_entry_t;

int sysreg_exception_handler(vm_vcpu_t *vcpu, uint32_t hsr);
