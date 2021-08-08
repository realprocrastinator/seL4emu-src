#pragma once

#include <emu/emu_types.h>
#include <emu/emu_common.h>

/**
 * This the message structure for emulating the seL4 IPC, the mesassage has
 * several types, using tag to distinguish them, if the mesassage uses the
 * seL4_sys_emu tag, then the content emulates the registers when the real seL4
 * syscall call happen. The registers order follows the syscall calling
 * convention and is architectual dependent. Here we use the x86_64 calling
 * convention.
 */

struct seL4emu_ipc_message {
  /**
   * tag specifies whether this IPC messgae is for the internal usage of the
   * emulation framework, or for emulating the seL4 IPC
   */
  seL4_Word tag;
  /* How many words needs to be passed */
  seL4_Word len;
  /* Who send the message */
  seL4_Word id;

  /**
   * On x86 arch we have 24 registers in total and upon the context switch in
   * seL4, those registers will be saved on in the current thread's tcb
   * structure. On the other hand, a pointer to the IPC Buffer may also be
   * passed using seL4 IPC call. However, at the moment we just pass the whole
   * IPC Buffer content using sockets, so we don't deal with the pointer be
   * passed acrossing address space.
   * We follow the layout of registers defined in the
   * kernel/include/arch/x86/arch/64/mode/machine/registerset.h. Despite that we
   * only use at most 7 registers, but we reserve spaces for all 24 registers.
   */
  seL4_Word words[n_contextRegisters + sizeof(seL4_IPCBuffer)];
};

typedef struct seL4emu_ipc_message seL4emu_ipc_message_t;

static inline void seL4emu_set_ipc_register(seL4_Word word,
                                            seL4emu_ipc_message_t *msg,
                                            enum _register type) {
  assert(msg);
  msg->words[type] = word;
}

static inline seL4_Word seL4emu_get_ipc_register(seL4emu_ipc_message_t *msg,
                                                 enum _register type) {
  assert(msg);
  return msg->words[type];
}

static inline void seL4emu_set_ipc_data(seL4_Word word,
                                            seL4emu_ipc_message_t *msg,
                                            enum seL4emu_ipc_data type) {
  assert(msg);
  switch(type) {
  case TAG:
    msg->tag = word;
    break;
  case LEN:
    msg->len = word;
    break;
  case ID:
    msg->id = word;
    break;
  default:
    assert(!"Not supported data type");
  };
}

static inline seL4_Word seL4emu_get_ipc_data(seL4emu_ipc_message_t *msg,
                                                 enum seL4emu_ipc_data type) {
  assert(msg);
  seL4_Word ret;
  switch(type) {
  case TAG:
    ret = msg->tag;
    break;
  case LEN:
    ret = msg->len;
    break;
  case ID:
    ret = msg->id;
    break;
  default:
    assert(!"Not supported data type");
  };

  return ret;
}

int seL4emu_uds_recv(int fd, size_t len, void* msg);
int seL4emu_uds_send(int fd, size_t len, void* msg);