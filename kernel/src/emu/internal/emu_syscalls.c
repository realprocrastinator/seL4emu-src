// seL4
#include <object/structures.h>
#include <api/syscall.h>
#include <arch/kernel/traps.h>
// glibc
#include <unistd.h>
// emulation
#include <emu/emu_ipcmsg.h>
#include <emu/emu_syscalls.h>
#include <emu/emu_globalstate.h>

void seL4emu_handle_syscalls(seL4emu_ipc_message_t *msg) {
  // unmarshall the msg
  syscall_t syscall = (syscall_t)seL4emu_get_ipc_register(msg, RDX);
  seL4_CPtr cptr = seL4emu_get_ipc_register(msg, RDI);
  word_t msg_info = seL4emu_get_ipc_register(msg, RSI);
  pid_t pid = (pid_t)seL4emu_get_ipc_data(msg, ID);

  printf("From=%ld, tag=%lu, len=%lu, sys=%ld, dest/capreg=%lu, msginfo=%lu, msg0=%lu, msg1=%lu, msg2=%lu, msg3=%lu\n", msg->id, msg->tag, msg->len, syscall, cptr, msg_info, msg->words[R10], msg->words[R8], msg->words[R9], msg->words[R15]);

  // In the real seL4 system, the `NODE_STATE(ksCurThread)` always points to the current running thread.
  // And in trap.S, we use LOAD_USER_CONTEXT, to point the `rsp` to the use current `tcb`, then push the registers
  // to the `tcb`, here we do the same. But because at the moment, we don't track which is the current running thread,
  // so we need to update this manually.
  tcb_t *curtcb = seL4emu_bk_tcbptr_get(pid);
  NODE_STATE(ksCurThread) = curtcb;
  assert(NODE_STATE(ksCurThread));

  /* copy ipc buffer to the tcb structure */
  

  // stash the seL4 threads data
  seL4emu_store_user_context(curtcb, msg->words);

  // call c_handle_syscall
  c_handle_syscall(cptr, msg_info, syscall);
}