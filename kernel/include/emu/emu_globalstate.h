#pragma once

#include <assert.h>
#include <autoconf.h>
#include <object/structures.h>
#include <unistd.h>

// TODO(Jiawei): move to the build system to auto gen based on the configuration
#define CONFIG_MAX_SEL4_CLIENTS 2

/* Bookkeeping data structure to track all the running seL4 threads on the
 * current host */
extern char seL4emu_g_tcbs[CONFIG_MAX_SEL4_CLIENTS][BIT(seL4_TCBBits)] ALIGN(
    BIT(TCB_SIZE_BITS));

/**
 * Bookkeeping data structure for tracking the seL4 clients using pids, as we
 * map each seL4 thread to a Linux process. So that we can distinguish the which
 * client is requesting the kernel service.
 */
extern int seL4emu_g_ids[CONFIG_MAX_SEL4_CLIENTS];

/**
 * TODO(Jiawei:) static allocate here at the moement for easy debugging and
 * continue the reset of the work Shouldn't do that way once the emulated
 * untyped memory is implemented. Static data structure of seL4 clients'
 * ipcBuffers, total size = 4k * CONFIG_MAX_SEL4_CLIENTS
 */
extern char seL4emu_g_ipc_buffers[CONFIG_MAX_SEL4_CLIENTS]
                                 [BIT(seL4_PageBits)] ALIGN(BIT(seL4_PageBits));

/**
 * TODO(Jiawei:) static allocate here at the moement for easy debugging and
 * continue the reset of the work Shouldn't do that way once the emulated
 * untyped memory is implemented. Static data structure of seL4 roottask's
 * bootinfo frame, total size = 4k
 */
extern char seL4emu_g_bi_frame[BIT(seL4_PageBits)] ALIGN(BIT(seL4_PageBits));

/**
 * Bookkeeping data structure for tracking the seL4 clients using pids, as we
 * map each seL4 thread to a Linux process. So that we can distinguish the which
 * client is requesting the kernel service.
 */
extern int seL4emu_g_ids[CONFIG_MAX_SEL4_CLIENTS];

/**
 * find the id mapped to the pid
 * TODO(Jiawei): avoid liner search for performance, but shoudld be fine as we
 * don't have many clients at the moment. return -1 if not found.
 */
static inline int seL4emu_bk_tid_get(pid_t pid) {
  assert(pid > 0);
  for (int i = 0; i < CONFIG_MAX_SEL4_CLIENTS; ++i) {
    if (seL4emu_g_ids[i] == pid) {
      return i;
    }
  }
  return -1;
}

/**
 * initialize the id list to all -1 indicating not used.
 */
static inline void seL4emu_bk_tid_init() {
  for (int i = 0; i < CONFIG_MAX_SEL4_CLIENTS; ++i) {
    seL4emu_g_ids[i] = -1;
  }
}

/**
 * find the first avalaible slot to insert an id
 * TODO(Jiawei): avoid liner search for performance, but shoudld be fine as we
 * don't have many clients at the moment. return -1 if all full, return slot idx
 * on success.
 */
static inline int seL4emu_bk_tid_insert(pid_t pid) {
  assert(pid > 0);
  int found = -1;
  for (int i = 0; i < CONFIG_MAX_SEL4_CLIENTS; ++i) {
    if (seL4emu_g_ids[i] == -1) {
      found = i;
      break;
    }
  }

  if (found >= 0) {
    seL4emu_g_ids[found] = pid;
  }

  return found;
}

/**
 * remove the pid from the id list
 * TODO(Jiawei): avoid liner search for performance, but shoudld be fine as we
 * don't have many clients at the moment. return -1 if pid not found, return
 * slot idx on success.
 */
static inline int seL4emu_bk_tid_remove(pid_t pid) {
  assert(pid > 0);
  int found = -1;
  for (int i = 0; i < CONFIG_MAX_SEL4_CLIENTS; ++i) {
    if (seL4emu_g_ids[i] == pid) {
      seL4emu_g_ids[i] = -1;
      found = i;
      break;
    }
  }

  return found;
}

/**
 * get the tcb structure belongs to the client
 * TODO(Jiawei): avoid liner search for performance, but shoudld be fine as we
 * don't have many clients at the moment. return 0 if not found, return address
 * found on success.
 */
word_t seL4emu_bk_tcbblock_get(pid_t pid);

/**
 * get the tcb ptr by given pid
 * return NULL if not found, return ptr to the tcb on success.
 */
tcb_t *seL4emu_bk_tcbptr_get(pid_t pid);

/**
 * stash n registers passing from the client to its tcb structure,
 * this emulates the push behaviour when context switch from user to kernel
 * in real seL4.
 */
void seL4emu_store_user_context(tcb_t *tcb, word_t *regs);