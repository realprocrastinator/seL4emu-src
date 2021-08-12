// seL4
#include <api/syscall.h>
#include <arch/kernel/traps.h>
#include <object/structures.h>
// glibc
#include <stdio.h>
#include <unistd.h>
// emulation
#include <emu/emu_common.h>
#include <emu/emu_globalstate.h>
#include <emu/emu_ipcmsg.h>
#include <emu/emu_svc.h>

static int handle_seL4syscalls(int sock, seL4emu_ipc_message_t *msg);
static int handle_internal_request(int sock, seL4emu_ipc_message_t *msg);
static int handle_send_client_hello(int sock);

int seL4emu_handle_client_ipc(int sock, seL4emu_ipc_message_t *msg) {
  // get msg tag
  int ret;
  switch (msg->tag) {
  case IPC_INTERNAL:
    ret = handle_internal_request(sock, msg);
    break;
  case IPC_SEL4:
    ret = handle_seL4syscalls(sock, msg);
    break;
  default:
    // should never reach here
    fprintf(stderr, "Invalid tag!\n");
    assert(!"Implementation bug!\n");
    break;
  }

  return ret;
}

static int handle_internal_request(int sock, seL4emu_ipc_message_t *msg) {
  assert(msg->tag == IPC_INTERNAL);
  assert(sock > 0);

  int ret;

  switch (msg->words[0]) {
  case SEL4EMU_INTERNAL_CLIENT_HELLO:
    ret = handle_send_client_hello(sock);
    break;
  case SEL4EMU_INTERNAL_INIT_SUCCESS:
    printf("Roottask init succeed.\n");
    break;
  case SEL4EMU_INTERNAL_INIT_FAILED:
    printf("Roottask init failed.\n");
    //TODO(Jiawei): end server loop and shutdown all clients;
    break;
  default:
    assert(!"Not supported yet!");
    break;
  }

  return ret;
}

static int handle_seL4syscalls(int sock, seL4emu_ipc_message_t *msg) {
  assert(msg->tag == IPC_SEL4);

  // unmarshall the msg
  syscall_t syscall = (syscall_t)seL4emu_get_ipc_register(msg, RDX);
  seL4_CPtr cptr = seL4emu_get_ipc_register(msg, RDI);
  word_t msg_info = seL4emu_get_ipc_register(msg, RSI);
  pid_t pid = (pid_t)seL4emu_get_ipc_data(msg, ID);

  // DEBUG printing
  // fprintf(stdout, "From=%ld, tag=%lu, len=%lu, sys=%ld, dest/capreg=%lu, msginfo=%lu, msg0=%lu, msg1=%lu, "
  //        "msg2=%lu, msg3=%lu, seL4_GetIPCBuffer->caps_or_badges=%lu\n",
  //        msg->id, msg->tag, msg->len, syscall, cptr, msg_info, msg->words[R10], msg->words[R8],
  //        msg->words[R9], msg->words[R15], xcap);

  // In the real seL4 system, the `NODE_STATE(ksCurThread)` always points to the current running
  // thread. And in trap.S, we use LOAD_USER_CONTEXT, to point the `rsp` to the use current `tcb`,
  // then push the registers to the `tcb`, here we do the same. But because at the moment, we don't
  // track which is the current running thread, so we need to update this manually.
  tcb_t *curtcb = seL4emu_bk_tcbptr_get(pid);
  NODE_STATE(ksCurThread) = curtcb;
  assert(NODE_STATE(ksCurThread));

  // copy ipc buffer to the tcb structure

  // stash the seL4 threads data
  seL4emu_store_user_context(curtcb, msg->words);

  // call c_handle_syscall
  c_handle_syscall(cptr, msg_info, syscall);

  // the fault is placed in the tcb, if we return here, we need to check if 
  // we should resume the current thread or not by checking the thread state!

  // TODO(Jiawei): as the scheduler is not implemented yet, we just resume!


  // emulates restoring the user context
  seL4emu_ipc_message_t reply;
  reply.tag = IPC_INTERNAL;
  memcpy(reply.words, curtcb->tcbArch.tcbContext.registers ,sizeof(word_t) * n_contextRegisters);

  if (seL4emu_uds_send(sock, sizeof(reply), &reply) < 0) {
    fprintf(stderr, "Failed to send bootinfo\n.");
    return -1;
  }

  return 0;
}

static int handle_send_client_hello(int sock) {
  seL4emu_ipc_message_t msg;

  // marshal the data
  // words[1] is the bootvaddr that we decided after parsing the ELF file, let client try map!
  // words[2] is the fd for the file backed shared memory
  // words[3] is the offset on the emulated physical memory where the content resides
  msg.tag = IPC_INTERNAL;
  msg.len = 4;

  void *emu_pbase = EMUST_GET_PMEM_BASEPTR(seL4emu_g_sysstate);
  word_t ipc_paddr = EMUST_GET_EMUP_RT_IPCBUFREF(seL4emu_g_sysstate);

  msg.words[0] = SEL4EMU_INTERNAL_BOOTINFO;
  msg.words[1] = EMUST_GET_RT_IPCBUFREF(seL4emu_g_sysstate);
  msg.words[2] = EMUST_GET_PMEMFD(seL4emu_g_sysstate);
  msg.words[3] = EMUST_PMEM_OFFSET(ipc_paddr, emu_pbase);
  
  if (seL4emu_uds_send(sock, sizeof(msg), &msg) < 0) {
    fprintf(stderr, "Failed to send bootinfo\n.");
    return -1;
  }

  return 0;
}