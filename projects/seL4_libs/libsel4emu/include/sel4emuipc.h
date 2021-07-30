#pragma once

#include <sel4/types.h>
#include <assert.h>
#include <stddef.h>

/* copy from kernel/include/arch/x86/64/mode/machine/registerset.h */

/* These are the indices of the registers in the
 * saved thread context. The values are determined
 * by the order in which they're saved in the trap
 * handler.
 *
 * BEWARE:
 * You will have to adapt traps.S extensively if
 * you change anything in this enum!
 */

/* This register layout is optimized for usage with
 * the syscall and sysret instructions. Interrupts
 * and sysenter have to do some juggling to make
 * things work */
enum _register {
    // User registers that will be preserved during syscall
    // Deliberately place the cap and badge registers early
    // So that when popping on the fastpath we can just not
    // pop these
    RDI                     = 0,    /* 0x00 */
    capRegister             = 0,
    badgeRegister           = 0,
    RSI                     = 1,    /* 0x08 */
    msgInfoRegister         = 1,
    RAX                     = 2,    /* 0x10 */
    RBX                     = 3,    /* 0x18 */
    RBP                     = 4,    /* 0x20 */
    R12                     = 5,    /* 0x28 */
#ifdef CONFIG_KERNEL_MCS
    replyRegister           = 5,
#endif
    R13                     = 6,    /* 0x30 */
#ifdef CONFIG_KERNEL_MCS
    nbsendRecvDest          = 6,
#endif
    R14                     = 7,    /* 0x38 */
    RDX                     = 8,    /* 0x40 */
    // Group the message registers so they can be efficiently copied
    R10                     = 9,    /* 0x48 */
    R8                      = 10,   /* 0x50 */
    R9                      = 11,   /* 0x58 */
    R15                     = 12,   /* 0x60 */
    FLAGS                   = 13,   /* 0x68 */
    // Put the NextIP, which is a virtual register, here as we
    // need to set this in the syscall path
    NextIP                  = 14,   /* 0x70 */
    // Same for the error code
    Error                   = 15,   /* 0x78 */
    /* Kernel stack points here on kernel entry */
    RSP                     = 16,   /* 0x80 */
    FaultIP                 = 17,   /* 0x88 */
    // Now user Registers that get clobbered by syscall
    R11                     = 18,   /* 0x90 */
    RCX                     = 19,   /* 0x98 */
    CS                      = 20,   /* 0xa0 */
    SS                      = 21,   /* 0xa8 */
    n_immContextRegisters   = 22,   /* 0xb0 */

    // For locality put these here as well
    FS_BASE                 = 22,   /* 0xb0 */
    TLS_BASE                = FS_BASE,
    GS_BASE                 = 23,   /* 0xb8 */

    n_contextRegisters      = 24    /* 0xc0 */
};

enum seL4emu_ipc_tag {
    ipc_sel4 = 0,
    ipc_internal  = 1
};

/**
 * This the message structure for emulating the seL4 IPC, the mesassage has several types, using tag 
 * to distinguish them, if the mesassage uses the seL4_sys_emu tag, then the content emulates the registers
 * when the real seL4 syscall call happen. The registers order follows the syscall calling convention and is
 * architectual dependent. Here we use the x86_64 calling convention.   
 */

struct seL4emu_ipc_message {
  /**
   * tag specifies whether this IPC messgae is for the internal usage of
   * the emulation framework, or for emulating the seL4 IPC
   */
  seL4_Word tag;
  /* How many words needs to be passed */
  seL4_Word len;
  /**
   * We follow the layout of registers defined in the kernel/include/arch/x86/arch/64/mode/machine/registerset.h.
   * Despite that we only use at most 7 registers, but we reserve spaces for all 25 registers. 
   */
  seL4_Word words[25];
};

typedef struct seL4emu_ipc_message seL4emu_ipc_message_t;

static inline void seL4emu_set_ipc_register(seL4_Word word, seL4emu_ipc_message_t *msg, enum _register type) {
  assert(msg);
  msg->words[type] = word;
}

void seL4emu_uds_send(seL4emu_ipc_message_t *msg, size_t len);
void seL4emu_uds_recv(seL4emu_ipc_message_t *msg, size_t len);