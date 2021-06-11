/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4/sel4.h>
#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_vm_util.h>
#include <sel4vm/arch/guest_arm_context.h>

#include "fault.h"

int vm_set_thread_context(vm_vcpu_t *vcpu, seL4_UserContext context)
{
    if (!fault_handled(vcpu->vcpu_arch.fault)) {
        /* If we are in a fault set its context directly as this will be
         * later synced */
        fault_set_ctx(vcpu->vcpu_arch.fault, &context);
    } else {
        /* Otherwise write to the TCB directly */
        seL4_CPtr tcb = vm_get_vcpu_tcb(vcpu);
        int err = seL4_TCB_WriteRegisters(tcb, false, 0, sizeof(context) / sizeof(context.pc), &context);
        if (err) {
            ZF_LOGE("Failed to set thread context: Unable to write TCB registers");
            return -1;
        }
    }
    return 0;
}

int vm_set_thread_context_reg(vm_vcpu_t *vcpu, unsigned int reg, uintptr_t value)
{
    seL4_CPtr tcb = vm_get_vcpu_tcb(vcpu);
    if (!fault_handled(vcpu->vcpu_arch.fault)) {
        /* If we are in a fault use and modify its cached context */
        seL4_UserContext *regs;
        regs = fault_get_ctx(vcpu->vcpu_arch.fault);
        (&regs->pc)[reg] = value;
    } else {
        /* Otherwise write to the TCB directly */
        seL4_UserContext regs;
        int err = seL4_TCB_ReadRegisters(tcb, false, 0, sizeof(regs) / sizeof(regs.pc), &regs);
        if (err) {
            ZF_LOGE("Failed to set thread context reg: Unable to read TCB registers");
            return -1;
        }
        (&regs.pc)[reg] = value;
        err = seL4_TCB_WriteRegisters(tcb, false, 0, sizeof(regs) / sizeof(regs.pc), &regs);
        if (err) {
            ZF_LOGE("Failed to set thread context register: Unable to write new register to TCB");
            return -1;
        }
    }
    return 0;
}

int vm_get_thread_context(vm_vcpu_t *vcpu, seL4_UserContext *context)
{
    if (!fault_handled(vcpu->vcpu_arch.fault)) {
        /* If we are in a fault use its cached context */
        seL4_UserContext *regs;
        regs = fault_get_ctx(vcpu->vcpu_arch.fault);
        *context = *regs;
    } else {
        /* Otherwise read the TCB directly */
        seL4_UserContext regs;
        seL4_CPtr tcb = vm_get_vcpu_tcb(vcpu);
        int err = seL4_TCB_ReadRegisters(tcb, false, 0, sizeof(regs) / sizeof(regs.pc), &regs);
        if (err) {
            ZF_LOGE("Failed to get thread context: Unable to read TCB registers");
            return -1;
        }
        *context = regs;
    }
    return 0;
}

int vm_get_thread_context_reg(vm_vcpu_t *vcpu, unsigned int reg, uintptr_t *value)
{
    if (!fault_handled(vcpu->vcpu_arch.fault)) {
        /* If we are in a fault use its cached context */
        seL4_UserContext *regs;
        regs = fault_get_ctx(vcpu->vcpu_arch.fault);
        *value = (&regs->pc)[reg];
    } else {
        /* Otherwise read it from the TCB directly */
        seL4_UserContext regs;
        seL4_CPtr tcb = vm_get_vcpu_tcb(vcpu);
        int err = seL4_TCB_ReadRegisters(tcb, false, 0, sizeof(regs) / sizeof(regs.pc), &regs);
        if (err) {
            ZF_LOGE("Failed to get thread context register: Unable to read TCB registers");
            return -1;
        }
        *value = (&regs.pc)[reg];
    }
    return 0;
}

int vm_set_arm_vcpu_reg(vm_vcpu_t *vcpu, seL4_Word reg, uintptr_t value)
{
    int err = seL4_ARM_VCPU_WriteRegs(vcpu->vcpu.cptr, reg, value);
    if (err) {
        ZF_LOGE("Failed to set VCPU register: Write registers failed");
        return -1;
    }
    return 0;
}

int vm_get_arm_vcpu_reg(vm_vcpu_t *vcpu, seL4_Word reg, uintptr_t *value)
{
    seL4_ARM_VCPU_ReadRegs_t res = seL4_ARM_VCPU_ReadRegs(vcpu->vcpu.cptr, reg);
    if (res.error) {
        ZF_LOGF("Read registers failed");
        return -1;
    }
    *value = res.value;
    return 0;
}
