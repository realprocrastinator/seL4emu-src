#include <autoconf.h>
#include <emu/emu_globalstate.h>
#include <machine/registerset.h>
#include <object/structures.h>
#include <plat_mode/machine/hardware.h>

// TODO(Jiawei): move to the build system to auto gen based on the configuration
// #define CONFIG_MAX_SEL4_CLIENTS 2

/* x86 specific */

/**
 * Used when virtual addressing is enabled, hence when the PG bit is set in CR0. CR3 enables the
 * processor to translate linear addresses into physical addresses by locating the page directory
 * and page tables for the current task. Typically, the upper 20 bits of CR3 become the page
 * directory base register (PDBR), which stores the physical address of the first page directory
 * entry. If the PCIDE bit in CR4 is set, the lowest 12 bits are used for the process-context
 * identifier (PCID).[1]
 * For the emulation the upper 20 bits points to the emulated physical memory address.
 */

// uint32_t seL4emu_g_cr3;

/**
 * Symbols used by bootcode, for the emulation, we just fake those values
 * at the moment
 */
char *ki_boot_end;
char *ki_end;
char *ki_skim_start;
char *ki_skim_end;

/* Bookkeeping data structure to track all the running seL4 threads on the current host */
tcb_t *seL4emu_g_tcbs[CONFIG_MAX_SEL4_CLIENTS];

/**
 * Bookkeeping data structure for tracking the seL4 clients using pids, as we map
 * each seL4 thread to a Linux process. So that we can distinguish the which client is
 * requesting the kernel service.
 */
int seL4emu_g_ids[CONFIG_MAX_SEL4_CLIENTS];

/**
 * TODO(Jiawei:) static allocate here at the moement for easy debugging and continue the reset of
 * the work Shouldn't do that way once the emulated untyped memory is implemented. Static data
 * structure of seL4 clients' ipcBuffers, total size = 4k * CONFIG_MAX_SEL4_CLIENTS
 */
char seL4emu_g_ipc_buffers[CONFIG_MAX_SEL4_CLIENTS][BIT(seL4_PageBits)] ALIGN(BIT(seL4_PageBits));

/**
 * TODO(Jiawei:) static allocate here at the moement for easy debugging and continue the reset of
 * the work Shouldn't do that way once the emulated untyped memory is implemented. Static data
 * structure of seL4 roottask's bootinfo frame, total size = 4k
 */
char seL4emu_g_bi_frame[BIT(seL4_PageBits)] ALIGN(BIT(seL4_PageBits));

tcb_t *seL4emu_bk_tcbptr_get(pid_t pid) {
  assert(pid > 0);
  tcb_t *ret;
  int id = seL4emu_bk_tid_get(pid);
  if (id < 0) {
    ret = NULL;
  } else {
    ret = seL4emu_g_tcbs[id];
  }
  return ret;
}

int seL4emu_bk_tcbptr_insert(pid_t pid, tcb_t *tcbptr) {
  assert(pid > 0);
  int ret = 0;
  int id = seL4emu_bk_tid_insert(pid);
  if (id < 0) {
    ret = -1;
  } else {
    seL4emu_g_tcbs[id] = tcbptr;
  }
  return ret;
}

void seL4emu_store_user_context(tcb_t *tcb, word_t *regs) {
  assert(tcb);
  assert(regs);

  /**
   * We strictly follow the layout of registers defined in machine specific registerset.h
   * the client side emulation framework also follows this layout
   */

  // -1 indicates this is a syscall
  setRegister(tcb, Error, -1);
  setRegister(tcb, NextIP, regs[NextIP]);
  setRegister(tcb, FLAGS, regs[FLAGS]);
  // message register
  setRegister(tcb, R15, regs[R15]);
  // message register
  setRegister(tcb, R9, regs[R9]);
  // message register
  setRegister(tcb, R8, regs[R8]);
  // message register
  setRegister(tcb, R10, regs[R10]);
  // syscall number
  setRegister(tcb, RDX, regs[RDX]);
  setRegister(tcb, R14, regs[R14]);
  setRegister(tcb, R13, regs[R13]);
  setRegister(tcb, R12, regs[R12]);
  setRegister(tcb, RBP, regs[RBP]);
  setRegister(tcb, RBX, regs[RBX]);
  setRegister(tcb, RAX, regs[RAX]);
  // msgInfo register
  setRegister(tcb, RSI, regs[RSI]);
  // capRegister
  setRegister(tcb, RDI, regs[RDI]);
}