/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/* x86 fetch/decode/emulate code

Author: W.A.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_ram.h>
#include <sel4vm/arch/guest_x86_context.h>

#include "sel4vm/guest_memory.h"

#include "processor/platfeature.h"
#include "processor/decode.h"
#include "guest_state.h"

/* TODO are these defined elsewhere? */
#define IA32_PDE_SIZE(pde) (pde & BIT(7))
#define IA32_PDE_PRESENT(pde) (pde & BIT(0))
#define IA32_PTE_ADDR(pte) (pte & 0xFFFFF000)
#define IA32_PSE_ADDR(pde) (pde & 0xFFC00000)

#define IA32_OPCODE_S(op) (op & BIT(0))
#define IA32_OPCODE_D(op) (op & BIT(1))
#define IA32_OPCODY_BODY(op) (op & 0b11111100)
#define IA32_MODRM_REG(m) ((m & 0b00111000) >> 3)

#define SEG_MULT (0x10)

enum decode_instr {
    DECODE_INSTR_MOV,
    DECODE_INSTR_MOVQ,
    DECODE_INSTR_INVALID
};

enum decode_prefix {
    ES_SEG_OVERRIDE = 0x26,
    CS_SEG_OVERRIDE = 0x2e,
    SS_SEG_OVERRIDE = 0x36,
    DS_SEG_OVERRIDE = 0x3e,
    FS_SEG_OVERRIDE = 0x64,
    GS_SEG_OVERRIDE = 0x65,
    OP_SIZE_OVERRIDE = 0x66,
    ADDR_SIZE_OVERRIDE = 0x67
};

struct x86_op {
    int reg;
    uint32_t val;
    size_t len;
};

struct decode_op {
    int curr_byte;
    uint8_t *instr;
    size_t instr_len;
    struct x86_op op;
};

struct decode_table {
    enum decode_instr instr;
    void (*decode_fn)(struct decode_op *);
};

static void debug_print_instruction(uint8_t *instr, int instr_len);

static void decode_modrm_reg_op(struct decode_op *decode_op)
{
    /* Mov with register */
    uint8_t modrm = decode_op->instr[decode_op->curr_byte];
    decode_op->curr_byte++;
    decode_op->op.reg = IA32_MODRM_REG(modrm);
    return;
}

static void decode_imm_op(struct decode_op *decode_op)
{
    /* Mov with immediate */
    decode_op->op.reg = -1;
    uint32_t immediate = 0;
    for (int j = 0; j < decode_op->op.len; j++) {
        immediate <<= 8;
        immediate |= decode_op->instr[decode_op->instr_len - j - 1];
    }
    decode_op->op.val = immediate;
    return;
}

static void decode_invalid_op(struct decode_op *decode_op)
{
    ZF_LOGE("can't emulate instruction!\n");
    debug_print_instruction(decode_op->instr, decode_op->instr_len);
    assert(0);
}

static const struct decode_table decode_table_1op[] = {
    [0 ... MAX_INSTR_OPCODES] = {DECODE_INSTR_INVALID, decode_invalid_op},
    [0x88] = {DECODE_INSTR_MOV, decode_modrm_reg_op},
    [0x89] = {DECODE_INSTR_MOV, decode_modrm_reg_op},
    [0x8a] = {DECODE_INSTR_MOV, decode_modrm_reg_op},
    [0x8b] = {DECODE_INSTR_MOV, decode_modrm_reg_op},
    [0x8c] = {DECODE_INSTR_MOV, decode_modrm_reg_op},
    [0xc6] = {DECODE_INSTR_MOV, decode_imm_op},
    [0xc7] = {DECODE_INSTR_MOV, decode_imm_op}
};

static const struct decode_table decode_table_2op[] = {
    [0 ... MAX_INSTR_OPCODES] = {DECODE_INSTR_INVALID, decode_invalid_op},
    [0x6f] = {DECODE_INSTR_MOVQ, decode_modrm_reg_op}
};

/* Get a word from a guest physical address */
inline static uint32_t guest_get_phys_word(vm_t *vm, uintptr_t addr)
{
    uint32_t val;

    vm_ram_touch(vm, addr, sizeof(uint32_t),
                 vm_guest_ram_read_callback, &val);

    return val;
}

/* Fetch a guest's instruction */
int vm_fetch_instruction(vm_vcpu_t *vcpu, uint32_t eip, uintptr_t cr3,
                         int len, uint8_t *buf)
{
    /* Walk page tables to get physical address of instruction */
    uintptr_t instr_phys = 0;

    /* ensure that PAE is not enabled */
    if (vm_guest_state_get_cr4(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr) & X86_CR4_PAE) {
        ZF_LOGE("Do not support walking PAE paging structures");
        return -1;
    }

    // TODO implement page-boundary crossing properly
    assert((eip >> 12) == ((eip + len) >> 12));

    uint32_t pdi = eip >> 22;
    uint32_t pti = (eip >> 12) & 0x3FF;

    uint32_t pde = guest_get_phys_word(vcpu->vm, cr3 + pdi * 4);

    assert(IA32_PDE_PRESENT(pde)); /* WTF? */

    if (IA32_PDE_SIZE(pde)) {
        /* PSE is used, 4M pages */
        instr_phys = (uintptr_t)IA32_PSE_ADDR(pde) + (eip & 0x3FFFFF);
    } else {
        /* 4k pages */
        uint32_t pte = guest_get_phys_word(vcpu->vm,
                                           (uintptr_t)IA32_PTE_ADDR(pde) + pti * 4);

        assert(IA32_PDE_PRESENT(pte));

        instr_phys = (uintptr_t)IA32_PTE_ADDR(pte) + (eip & 0xFFF);
    }

    /* Fetch instruction */
    vm_ram_touch(vcpu->vm, instr_phys, len,
                 vm_guest_ram_read_callback, buf);

    return 0;
}

/* Returns 1 if this byte is an x86 instruction prefix */
static int is_prefix(uint8_t byte)
{
    switch (byte) {
    case ES_SEG_OVERRIDE:
    case CS_SEG_OVERRIDE:
    case SS_SEG_OVERRIDE:
    case DS_SEG_OVERRIDE:
    case FS_SEG_OVERRIDE:
    case GS_SEG_OVERRIDE:
    case ADDR_SIZE_OVERRIDE:
    case OP_SIZE_OVERRIDE:
        return 1;
    }

    return 0;
}

static void debug_print_instruction(uint8_t *instr, int instr_len)
{
    printf("instruction dump: ");
    for (int j = 0; j < instr_len; j++) {
        printf("%2x ", instr[j]);
    }
    printf("\n");
}

/* Partial support to decode an instruction for a memory access
   This is very crude. It can break in many ways. */
int vm_decode_instruction(uint8_t *instr, int instr_len, int *reg, uint32_t *imm, int *op_len)
{
    struct decode_op dec_op;
    dec_op.instr = instr;
    dec_op.instr_len = instr_len;
    dec_op.op.len = 1;
    /* First loop through and check prefixes */
    int i;
    for (i = 0; i < instr_len; i++) {
        if (is_prefix(instr[i])) {
            if (instr[i] == OP_SIZE_OVERRIDE) {
                /* 16 bit modifier */
                dec_op.op.len = 2;
            }
        } else {
            /* We've hit the opcode */
            break;
        }
    }

    dec_op.curr_byte = i;
    assert(dec_op.curr_byte < instr_len); /* We still need an opcode */

    uint8_t opcode = instr[dec_op.curr_byte];
    dec_op.curr_byte++;
    if (opcode == OP_ESCAPE) {
        opcode = instr[dec_op.curr_byte];
        dec_op.curr_byte++;
        decode_table_2op[opcode].decode_fn(&dec_op);
    } else {
        decode_table_1op[opcode].decode_fn(&dec_op);
    }

    if (dec_op.op.len != 2 && IA32_OPCODE_S(opcode)) {
        dec_op.op.len = 4;
    }

    *reg = dec_op.op.reg;
    *imm = dec_op.op.val;
    *op_len = dec_op.op.len;
    return 0;
}

void vm_decode_ept_violation(vm_vcpu_t *vcpu, int *reg, uint32_t *imm, int *size)
{
    /* Decode instruction */
    uint8_t ibuf[15];
    int instr_len = vm_guest_exit_get_int_len(vcpu->vcpu_arch.guest_state);
    vm_fetch_instruction(vcpu,
                         vm_guest_state_get_eip(vcpu->vcpu_arch.guest_state),
                         vm_guest_state_get_cr3(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr),
                         instr_len, ibuf);

    vm_decode_instruction(ibuf, instr_len, reg, imm, size);
}

/*
   Useful information: The GDT loaded by the Linux SMP trampoline looks like:
0x00: 00 00 00 00 00 00 00 00
0x08: 00 00 00 00 00 00 00 00
0x10: ff ff 00 00 00 9b cf 00 <- Executable 0x00000000-0xffffffff
0x18: ff ff 00 00 00 93 cf 00 <- RW data    0x00000000-0xffffffff
*/

/* Interpret just enough virtual 8086 instructions to run trampoline code.
   Returns the final jump address */
uintptr_t vm_emulate_realmode(vm_vcpu_t *vcpu, uint8_t *instr_buf,
                              uint16_t *segment, uintptr_t eip, uint32_t len, guest_state_t *gs)
{
    /* We only track one segment, and assume that code and data are in the same
       segment, which is valid for most trampoline and bootloader code */
    uint8_t *instr = instr_buf;
    assert(segment);

    while (instr - instr_buf < len) {
        uintptr_t mem = 0;
        uint32_t lit = 0;
        int m66 = 0;

        uint32_t base = 0;
        uint32_t limit = 0;

        if (*instr == 0x66) {
            m66 = 1;
            instr++;
        }

        if (*instr == 0x0f) {
            instr++;
            if (*instr == 0x01) {
                instr++;
                if (*instr == 0x1e) {
                    // lidtl
                    instr++;
                    memcpy(&mem, instr, 2);
                    mem += *segment * SEG_MULT;
                    instr += 2;

                    /* Limit is first 2 bytes, base is next 4 bytes */
                    vm_ram_touch(vcpu->vm, mem,
                                 2, vm_guest_ram_read_callback, &limit);
                    vm_ram_touch(vcpu->vm, mem + 2,
                                 4, vm_guest_ram_read_callback, &base);
                    ZF_LOGD("lidtl %p\n", (void *)mem);

                    vm_guest_state_set_idt_base(gs, base);
                    vm_guest_state_set_idt_limit(gs, limit);
                } else if (*instr == 0x16) {
                    // lgdtl
                    instr++;
                    memcpy(&mem, instr, 2);
                    mem += *segment * SEG_MULT;
                    instr += 2;

                    /* Limit is first 2 bytes, base is next 4 bytes */
                    vm_ram_touch(vcpu->vm, mem,
                                 2, vm_guest_ram_read_callback, &limit);
                    vm_ram_touch(vcpu->vm, mem + 2,
                                 4, vm_guest_ram_read_callback, &base);
                    ZF_LOGD("lgdtl %p; base = %x, limit = %x\n", (void *)mem,
                            base, limit);

                    vm_guest_state_set_gdt_base(gs, base);
                    vm_guest_state_set_gdt_limit(gs, limit);
                } else {
                    //ignore
                    instr++;
                }
            } else {
                //ignore
                instr++;
            }
        } else if (*instr == 0xea) {
            /* Absolute jmp */
            instr++;
            uint32_t base = 0;
            uintptr_t jmp_addr = 0;
            if (m66) {
                // base is 4 bytes
                /* Make the wild assumptions that we are now in protected mode
                   and the relevant GDT entry just covers all memory. Therefore
                   the base address is our absolute address. This just happens
                   to work with Linux and probably other modern systems that
                   don't use the GDT much. */
                memcpy(&base, instr, 4);
                instr += 4;
                jmp_addr = base;
                memcpy(segment, instr, 2);
            } else {
                memcpy(&base, instr, 2);
                instr += 2;
                memcpy(segment, instr, 2);
                jmp_addr = *segment * SEG_MULT + base;
            }
            instr += 2;
            ZF_LOGD("absolute jmpf $%p, cs now %04x\n", (void *)jmp_addr, *segment);
            if (((int64_t)jmp_addr - (int64_t)(len + eip)) >= 0) {
                vm_guest_state_set_cs_selector(gs, *segment);
                return jmp_addr;
            } else {
                instr = jmp_addr - eip + instr_buf;
            }
        } else {
            switch (*instr) {
            case 0xa1:
                /* mov offset memory to eax */
                instr++;
                memcpy(&mem, instr, 2);
                instr += 2;
                mem += *segment * SEG_MULT;
                ZF_LOGD("mov %p, eax\n", (void *)mem);
                uint32_t eax;
                vm_ram_touch(vcpu->vm, mem,
                             4, vm_guest_ram_read_callback, &eax);
                vm_set_thread_context_reg(vcpu, VCPU_CONTEXT_EAX, eax);
                break;
            case 0xc7:
                instr++;
                if (*instr == 0x06) { // modrm
                    int size;
                    instr++;
                    /* mov literal to memory */
                    memcpy(&mem, instr, 2);
                    mem += *segment * SEG_MULT;
                    instr += 2;
                    if (m66) {
                        memcpy(&lit, instr, 4);
                        size = 4;
                    } else {
                        memcpy(&lit, instr, 2);
                        size = 2;
                    }
                    instr += size;
                    ZF_LOGD("mov $0x%x, %p\n", lit, (void *)mem);
                    vm_ram_touch(vcpu->vm, mem,
                                 size, vm_guest_ram_write_callback, &lit);
                }
                break;
            case 0xba:
                //?????mov literal to dx
                /* ignore */
                instr += 2;
                break;
            case 0x8c:
            case 0x8e:
                /* mov to/from sreg. ignore */
                instr += 2;
                break;
            default:
                /* Assume this is a single byte instruction we can ignore */
                instr++;
            }
        }

        ZF_LOGI("read %zu bytes\n", (size_t)(instr - instr_buf));
    }

    return 0;
}
