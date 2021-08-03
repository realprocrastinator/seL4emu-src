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
#include <kernel/thread.h>
#include <model/statedata.h>
// emulation
#include <emu/emu_ipcmsg.h>
#include <emu/emu_syscalls.h>
#include <emu/emu_globalstate.h>

#define SOCKET_NAME "/tmp/uds-test.socket"
#define BUFFER_SIZE (27 * 8)

static void cleanup_onexit() {
  printf("Bye\n");
  unlink(SOCKET_NAME);
}

static bool_t create_idle_thread(void) {
  pptr_t pptr;

#ifdef ENABLE_SMP_SUPPORT
  for (int i = 0; i < CONFIG_MAX_NUM_NODES; i++) {
#endif /* ENABLE_SMP_SUPPORT */
    pptr = (pptr_t)&ksIdleThreadTCB[SMP_TERNARY(i, 0)];
    NODE_STATE_ON_CORE(ksIdleThread, i) = TCB_PTR(pptr + TCB_OFFSET);
    configureIdleThread(NODE_STATE_ON_CORE(ksIdleThread, i));
#ifdef CONFIG_DEBUG_BUILD
    setThreadName(NODE_STATE_ON_CORE(ksIdleThread, i), "idle_thread");
#endif
    SMP_COND_STATEMENT(NODE_STATE_ON_CORE(ksIdleThread, i)->tcbAffinity = i);
#ifdef CONFIG_KERNEL_MCS
    bool_t result = configure_sched_context(
        NODE_STATE_ON_CORE(ksIdleThread, i),
        SC_PTR(&ksIdleThreadSC[SMP_TERNARY(i, 0)]),
        usToTicks(CONFIG_BOOT_THREAD_TIME_SLICE * US_IN_MS), SMP_TERNARY(i, 0));
    SMP_COND_STATEMENT(
        NODE_STATE_ON_CORE(ksIdleThread, i)->tcbSchedContext->scCore = i;)
    if (!result) {
      printf("Kernel init failed: Unable to allocate sc for idle thread\n");
      return false;
    }
#endif
#ifdef ENABLE_SMP_SUPPORT
  }
#endif /* ENABLE_SMP_SUPPORT */
  return true;
}

static void init_boot(void) {
  create_idle_thread();
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