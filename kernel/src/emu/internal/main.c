#define _GNU_SOURCE
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

// seL4
#include <arch/kernel/boot.h>
#include <arch/kernel/vspace.h>
#include <kernel/boot.h>
#include <kernel/thread.h>
#include <machine.h>
#include <model/statedata.h>

// emulation
#include <emu/emu_globalstate.h>
#include <emu/emu_ipcmsg.h>
#include <emu/emu_syscalls.h>

#define SOCKET_NAME "/tmp/uds-test.socket"
#define BUFFER_SIZE (27 * 8)

static void cleanup_onexit() {
  printf("Bye\n");
  unlink(SOCKET_NAME);
}

// static bool_t create_idle_thread(void) {
//   pptr_t pptr;

// #ifdef ENABLE_SMP_SUPPORT
//   for (int i = 0; i < CONFIG_MAX_NUM_NODES; i++) {
// #endif /* ENABLE_SMP_SUPPORT */
//     pptr = (pptr_t)&ksIdleThreadTCB[SMP_TERNARY(i, 0)];
//     NODE_STATE_ON_CORE(ksIdleThread, i) = TCB_PTR(pptr + TCB_OFFSET);
//     configureIdleThread(NODE_STATE_ON_CORE(ksIdleThread, i));
// #ifdef CONFIG_DEBUG_BUILD
//     setThreadName(NODE_STATE_ON_CORE(ksIdleThread, i), "idle_thread");
// #endif
//     SMP_COND_STATEMENT(NODE_STATE_ON_CORE(ksIdleThread, i)->tcbAffinity = i);
// #ifdef CONFIG_KERNEL_MCS
//     bool_t result = configure_sched_context(
//         NODE_STATE_ON_CORE(ksIdleThread, i),
//         SC_PTR(&ksIdleThreadSC[SMP_TERNARY(i, 0)]),
//         usToTicks(CONFIG_BOOT_THREAD_TIME_SLICE * US_IN_MS), SMP_TERNARY(i, 0));
//     SMP_COND_STATEMENT(
//         NODE_STATE_ON_CORE(ksIdleThread, i)->tcbSchedContext->scCore = i;)
//     if (!result) {
//       printf("Kernel init failed: Unable to allocate sc for idle thread\n");
//       return false;
//     }
// #endif
// #ifdef ENABLE_SMP_SUPPORT
//   }
// #endif /* ENABLE_SMP_SUPPORT */
//   return true;
// }

static void init_boot(void) {

  cap_t root_cnode_cap;
  vptr_t extra_bi_frame_vptr;
  vptr_t bi_frame_vptr;
  // 0 indicating root server id
  vptr_t ipcbuf_vptr;
  cap_t it_vspace_cap;
  // cap_t         it_ap_cap;
  cap_t ipcbuf_cap;
  word_t extra_bi_size = sizeof(seL4_BootInfoHeader);
  // pptr_t        extra_bi_offset = 0;
  // uint32_t      tsc_freq;
  // create_frames_of_region_ret_t create_frames_ret;
  // create_frames_of_region_ret_t extra_bi_ret;

  // TODO(Jiawei): Fake data for testing! REMOVE THIS!
  ui_info_t ui_info;
  ui_info.p_reg.start = 10563584;
  ui_info.p_reg.end = 11759616;
  ui_info.pv_offset = 6369280;

  /* convert from physical addresses to kernel pptrs */
  // region_t ui_reg             = paddr_to_pptr_reg(ui_info.p_reg);
  // region_t boot_mem_reuse_reg = paddr_to_pptr_reg(boot_mem_reuse_p_reg);

  /* convert from physical addresses to userland vptrs */
  v_region_t ui_v_reg;
  v_region_t it_v_reg;
  ui_v_reg.start = ui_info.p_reg.start - ui_info.pv_offset;
  ui_v_reg.end = ui_info.p_reg.end - ui_info.pv_offset;
  ui_info.v_entry = 0;

  // set ipc buffer ptr
  ipcbuf_vptr = (vptr_t)seL4emu_g_ipc_buffers[0];
  bi_frame_vptr = ipcbuf_vptr + BIT(PAGE_BITS);
  extra_bi_frame_vptr = bi_frame_vptr + BIT(PAGE_BITS);

  word_t extra_bi_size_bits = calculate_extra_bi_size_bits(extra_bi_size);

  /* The region of the initial thread is the user image + ipcbuf and boot info */
  it_v_reg.start = ui_v_reg.start;
  it_v_reg.end = ROUND_UP(extra_bi_frame_vptr + BIT(extra_bi_size_bits), PAGE_BITS);

  // init free mem
  // arch_init_freemem(ui_info.p_reg, it_v_reg, mem_p_regs, extra_bi_size_bits);

  // create root cnode cap
  root_cnode_cap = create_root_cnode();

  // populate bi frame

  // create initial thread address space with enough virtual adresses
  // to cover the user image + ipc_buffer and bootinfo frames
  it_vspace_cap = create_it_address_space(root_cnode_cap, it_v_reg);
  if (cap_get_capType(it_vspace_cap) == cap_null_cap) {
    fprintf(stderr, "Failed to create initial address space.\n");
    exit(1);
  }

  // create and map extra boot info region

  // create initial thread's IPCBuffer cap

  // create frames for userland image frames

  // create the initial ASID pool

  // create idle thread

  if (!create_idle_thread()) {
    fprintf(stderr, "Failed to create idle thread.\n");
    exit(1);
  }

  // IOMMU set to disable

  // create initial thread
  tcb_t *initial_thread = create_initial_thread(root_cnode_cap, it_vspace_cap, ui_info.v_entry,
                                                bi_frame_vptr, ipcbuf_vptr, ipcbuf_cap);
  if (!initial_thread) {
    fprintf(stderr, "Failed to create inital thread.\n");
    exit(1);
  }

  // boot.c:init_core_state
  NODE_STATE(ksCurThread) = NODE_STATE(ksIdleThread);
}

static int init_ipc_socket(int *sock) {
  struct sockaddr_un name;
  int ret;

  // create local socket
  *sock = socket(AF_UNIX, SOCK_SEQPACKET, 0);

  if (*sock == -1) {
    perror("socket");
    goto socket_init_fail;
  }

  // clean the whole structure
  memset(&name, 0, sizeof(name));

  // bind socket to the socket name
  name.sun_family = AF_UNIX;
  strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

  ret = bind(*sock, (const struct sockaddr *)&name, sizeof(name));
  if (ret == -1) {
    perror("bind");
    goto socket_init_fail;
  }

  // prepare for acceptin connections. The backlog size is set to 20.
  // So while one request is being processed other requests can be waiting.

  ret = listen(*sock, 20);
  if (ret == -1) {
    perror("listen");
    goto socket_init_fail;
  }

  return 0;

socket_init_fail:
  return -1;
}

static void syscall_loop(int conn_sock) {
  int data_socket;
  seL4emu_ipc_message_t buffer;
  int ret;

  // this is the main loop for handling the connections
  for (;;) {
    // wait for incoming connections
    data_socket = accept(conn_sock, NULL, NULL);
    if (data_socket == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    // wait for the next data packet
    ret = read(data_socket, &buffer, sizeof(buffer));
    if (ret == -1) {
      perror("read failed");
      exit(EXIT_FAILURE);
    }

    seL4emu_handle_syscalls(&buffer);

    // close the socket
    close(data_socket);
  }
}

/* This program is only used for testing whether the build system works */

int main() {
  atexit(cleanup_onexit);

  int connection_socket;
  if (init_ipc_socket(&connection_socket) < 0) {
    exit(EXIT_FAILURE);
  }

  init_boot();
  seL4emu_bk_tid_init();

  pid_t pid = fork();
  if (pid > 0) {
    printf("Starting client %d.\n", pid);
    int id = seL4emu_bk_tid_insert(pid);
    if (id < 0) {
      printf("Failed to create a new seL4 thread due to runnning out of space in tid list.\n");
      goto cleanup_exit;
    }
  } else if (pid == 0) {
    execve("/home/kukuku/UNSW/cs9991/sel4-projects/sel4-emu-stub/"
           "hello-world_build/hello-world",
           NULL, NULL);
  }

  syscall_loop(connection_socket);

  int status;

cleanup_exit:
  waitpid(-1, &status, 0);

  close(connection_socket);
  // unlink the socket
  unlink(SOCKET_NAME);
}