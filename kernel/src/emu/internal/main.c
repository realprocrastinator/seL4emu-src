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

// emulation
#include <emu/emu_globalstate.h>
#include <emu/emu_ipcmsg.h>
#include <emu/emu_svc.h>
#include <emu_arch/emu_boot.h>

// seL4
#include <model/statedata.h>
#include <kernel/boot.h>

#define SOCKET_NAME "/tmp/uds-test.socket"
#define ROOT_TASK_PATH "/home/kukuku/UNSW/cs9991/sel4-projects/sel4-emu-stub/hello-world_build/emu-images/hello-world-image-x86_64-pc99-emu"

static void cleanup_onexit() {
  printf("Bye\n");
  unlink(SOCKET_NAME);
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

  // TODO(Jiawei): for easy testing, we use blocking socket, as we
  // only have one roottask at the moment. We shoudld use non-blocking
  // socket to avoid starving other clients later.

  // wait for incoming connections
  data_socket = accept(conn_sock, NULL, NULL);

  // this is the main loop for handling the connections
  for (;;) {
    if (data_socket == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    // wait for the next data packet
    ret = read(data_socket, &buffer, sizeof(buffer));
    if (ret == -1) {
      perror("read failed");
      exit(EXIT_FAILURE);
    } else if (ret == 0) {
      // EOF
      // socket closed on the client side
      close(data_socket);
      break;
    }

    seL4emu_handle_client_ipc(data_socket, &buffer);
  }
}

/* This program is only used for testing whether the build system works */

int main() {
  atexit(cleanup_onexit);

  /*
   * This is a fake memory region emulating the 4GB physical memory for testing
   * Should be using a file backed mapping later and let use configured teh size.
   */
  void *addr = mmap((void *)SEL4_EMU_PMEM_BASE, SEL4_EMU_PMEM_SIZE, PROT_READ | PROT_WRITE,
                    MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  assert((word_t)addr == SEL4_EMU_PMEM_BASE);
  if (addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  int connection_socket;
  if (init_ipc_socket(&connection_socket) < 0) {
    exit(EXIT_FAILURE);
  }

  /* Initialize the global threads tracking array */
  seL4emu_bk_tid_init();

  /* emulate system boot to get the bootinfo and intialize the system */
  seL4emu_boot();
  tcb_t *roottcb = NODE_STATE(ksCurThread);
  assert(roottcb != NULL);

  /* save the current emulation system state */
  seL4emu_g_sysstate.emu_pmem.base = addr;
  seL4emu_g_sysstate.emu_pmem.size = SEL4_EMU_PMEM_SIZE;
  // TODO(Jiawei): how much pmem left?
  seL4emu_g_sysstate.conn_sock = connection_socket;
  seL4emu_g_sysstate.seL4emu_curthread = roottcb;
  seL4emu_g_sysstate.seL4roottask = roottcb;
  seL4emu_g_sysstate.rt_obj = &rootserver;
  
  pid_t pid = fork();
  if (pid > 0) {
    printf("Starting client %d.\n", pid);
    
    /* At this stage, we al ready set up the roottask tcb if successfull */
    if (seL4emu_bk_tcbptr_insert(pid, roottcb) < 0) {
      printf("Failed to create a new seL4 thread due to runnning out of space in tid list.\n");
      goto cleanup_exit;
    }
    
  } else if (pid == 0) {
    /* pass the boot info to the roottask */
    execve(ROOT_TASK_PATH, NULL, NULL);
  }

  syscall_loop(connection_socket);

  int status;

cleanup_exit:
  waitpid(-1, &status, 0);
  munmap((void *)SEL4_EMU_PMEM_BASE, SEL4_EMU_PMEM_SIZE);
  close(connection_socket);
  // unlink the socket
  unlink(SOCKET_NAME);
}