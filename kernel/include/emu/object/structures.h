#pragma once

#include <autoconf.h>

/* A TCB CNode and a TCB are always allocated together, and adjacently.
 * The CNode comes first. */
enum emu_tcb_cnode_index {
  /* CSpace root */
  emu_tcbCTable = 0,

  /* VSpace root */
  emu_tcbVTable = 1,

#ifdef CONFIG_KERNEL_MCS
  /* IPC buffer cap slot */
  emu_tcbBuffer = 2,

  /* Fault endpoint slot */
  emu_tcbFaultHandler = 3,

  /* Timeout endpoint slot */
  emu_tcbTimeoutHandler = 4,
#else
  /* Reply cap slot */
  emu_tcbReply = 2,

  /* TCB of most recent IPC sender */
  emu_tcbCaller = 3,

  /* IPC buffer cap slot */
  emu_tcbBuffer = 4,
#endif
  emu_tcbCNodeEntries
};
typedef word_t tcb_cnode_index_t;

enum emu_asidSizeConstants {
  emu_asidHighBits = seL4_NumASIDPoolsBits,
  emu_asidLowBits = seL4_ASIDPoolIndexBits
};