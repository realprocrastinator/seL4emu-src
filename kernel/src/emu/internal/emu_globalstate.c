#include <autoconf.h>
#include <emu/emu_globalstate.h>
#include <machine/registerset.h>
#include <object/structures.h>

// TODO(Jiawei): move to the build system to auto gen based on the configuration
// #define CONFIG_MAX_SEL4_CLIENTS 2

/* Bookkeeping data structure to track all the running seL4 threads on the current host */
char seL4emu_g_tcbs[CONFIG_MAX_SEL4_CLIENTS][BIT(seL4_TCBBits)] ALIGN(BIT(TCB_SIZE_BITS));

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

word_t seL4emu_bk_tcbblock_get(pid_t pid) {
  assert(pid > 0);
  word_t ret;
  int id = seL4emu_bk_tid_get(pid);
  if (id < 0) {
    ret = 0;
  } else {
    ret = (word_t)&seL4emu_g_tcbs[id];
  }
  return ret;
}

tcb_t *seL4emu_bk_tcbptr_get(pid_t pid) {
  word_t vaddr = seL4emu_bk_tcbblock_get(pid);
  tcb_t *ret;

  if (!vaddr) {
    ret = NULL;
  } else {
    ret = TCB_PTR(vaddr + TCB_OFFSET);
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