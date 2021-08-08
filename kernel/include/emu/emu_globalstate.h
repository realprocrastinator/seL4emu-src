#pragma once

#include <assert.h>
#include <autoconf.h>
#include <kernel/boot.h>
#include <object/structures.h>
#include <unistd.h>

// TODO(Jiawei): move to the build system to auto gen based on the configuration
#define CONFIG_MAX_SEL4_CLIENTS 2
#define SEL4_EMU_PMEM_SIZE 4096UL * 1024UL * 1024UL
#define SEL4_EMU_PMEM_BASE 0xa000000UL

/* Bookkeeping data structure to track all the running seL4 threads on the
 * current host */
extern tcb_t *seL4emu_g_tcbs[CONFIG_MAX_SEL4_CLIENTS];

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
extern char seL4emu_g_ipc_buffers[CONFIG_MAX_SEL4_CLIENTS][BIT(seL4_PageBits)] ALIGN(BIT(seL4_PageBits));

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

struct seL4emu_pmem {
  int fd;
  size_t size;
  size_t reserve;
  size_t free;
  void *base;
};

/* The emulation internal state */
struct seL4emu_sysstate {
  pid_t children[CONFIG_MAX_SEL4_CLIENTS];
  struct seL4emu_pmem emu_pmem;
  tcb_t *seL4emu_curthread;
  tcb_t *seL4roottask;
  /* Where the actual objects of the roottask lives */
  rootserver_mem_t *rt_obj;
  /* Those are just dummy numbers passing to the roottask as hint to map */
  word_t rt_bi_vptr;
  word_t rt_ipc_vptr;
  word_t rt_xbi_vptr;
  int conn_sock;
};

typedef struct seL4emu_sysstate seL4emu_sysstate_t;

#define EMUST_GET_EMUP_RT_BIREF(state) (state.rt_obj->boot_info)
#define EMUST_GET_EMUP_RT_XBIREF(state) (state.rt_obj->extra_bi)
#define EMUST_GET_EMUP_RT_IPCBUFREF(state) (state.rt_obj->ipc_buf)

#define EMUST_GET_RT_BIREF(state) (state.rt_bi_vptr)
#define EMUST_GET_RT_XBIREF(state) (state.rt_xbi_vptr)
#define EMUST_GET_RT_IPCBUFREF(state) (state.rt_ipc_vptr)

#define EMUST_GET_PMEMFD(state) (state.emu_pmem.fd)
#define EMUST_GET_PMEM_BASEPTR(state) (state.emu_pmem.base)
#define EMUST_GET_PMEM_BASEREF(state) ((word_t)state.emu_pmem.base)

#define EMUST_SET_RT_BIREF(state, vaddr) (state.rt_bi_vptr = vaddr)
#define EMUST_SET_RT_XBIREF(state, vaddr) (state.rt_xbi_vptr = vaddr)
#define EMUST_SET_RT_IPCBUFREF(state, vaddr) (state.rt_ipc_vptr = vaddr)

#define EMUST_PMEM_OFFSET(addr, base) ((word_t)addr - (word_t)base)

extern struct seL4emu_sysstate seL4emu_g_sysstate;

/**
 * initialize the id list to all -1 indicating not used.
 */
static inline void seL4emu_bk_tid_init() {
  for (int i = 0; i < CONFIG_MAX_SEL4_CLIENTS; ++i) {
    seL4emu_g_ids[i] = -1;
  }
}

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
 * get the tcb ptr by given pid
 * return NULL if not found, return ptr to the tcb on success.
 */
tcb_t *seL4emu_bk_tcbptr_get(pid_t pid);

/**
 * set the tcb ptr by given pid
 * return -1 if failed, return 0 on success.
 */
int seL4emu_bk_tcbptr_insert(pid_t pid, tcb_t *tcbptr);

/**
 * stash n registers passing from the client to its tcb structure,
 * this emulates the push behaviour when context switch from user to kernel
 * in real seL4.
 */
void seL4emu_store_user_context(tcb_t *tcb, word_t *regs);