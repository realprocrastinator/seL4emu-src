#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

// emulation
#include <emu/emu_globalstate.h>
#include <emu/emu_ipcmsg.h>
#include <emu/emu_svc.h>
#include <emu_arch/emu_boot.h>

// seL4
#include <kernel/boot.h>
#include <model/statedata.h>

#define SOCKET_NAME "/tmp/uds-test.socket"
// #define ROOT_TASK_PATH
// "/home/kukuku/UNSW/cs9991/sel4-projects/sel4-emu-stub/ipc_build/emu-images/capdl-loader-image-x86_64-pc99-emu"s
// #define ROOT_TASK_PATH
// "/home/kukuku/UNSW/cs9991/sel4-projects/sel4-emu-stub/hello-world_build/emu-images/hello-world-image-x86_64-pc99-emu"
#define ROOT_TASK_PATH "/home/kukuku/UNSW/cs9991/sel4-projects/sel4-emu-stub/capabilities_build/emu-images/capabilities-image-x86_64-pc99-emu"

#define EMUPMEM_FILE "/seL4emu_pmem"

static void cleanup_onexit() {
  printf("Bye\n");
  unlink(SOCKET_NAME);
}

static int init_uds_socket(const char *path, int *sock) {
  struct sockaddr_un name;
  int ret;

  *sock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (*sock < 0) {
    perror("socket");
    return -1;
  }

  memset(&name, 0, sizeof(name));

  // bind socket to the socket name
  name.sun_family = AF_UNIX;
  strncpy(name.sun_path, path, sizeof(name.sun_path) - 1);

  // if the program is terminiated abnormally, we might haven't cleaned
  // up the UDS socket properly, so try clena it first, it might return
  // error if we already cleaned up.
  // TODO(Jiawei): Maybe try abstract socket, and let kernel do the job?
  if (unlink(path) < 0 && errno != ENOENT) {
    perror("unlink");
    return -1;
  }

  ret = bind(*sock, (const struct sockaddr *)&name, sizeof(name));
  if (ret < 0) {
    perror("bind");
    return -1;
  }

  // prepare for acceptin connections. The backlog size is set to CONFIG_MAX_SEL4_CLIENTS.
  // So while one request is being processed other requests can be waiting.
  ret = listen(*sock, CONFIG_MAX_SEL4_CLIENTS);
  if (ret < 0) {
    perror("listen");
    return -1;
  }

  return 0;
}

// We created a fixed base and fixed size share memory region to emulate
// the physical memory. The seL4 kernel emulator can use it as physical
// memory or the physical memory mapping in the kernel window. The client
// can map this to their address spaces and r/e to the specific offset.
static int init_sel4emu_pmem(const char *name, void *base, size_t size, int *outfd) {
  // as usual we try cleanup if program has crashed before, if case a clean up
  // hasn't been done yet.
  if (shm_unlink(name) < 0 && errno != ENOENT) {
    perror("shm_unlink");
    return -1;
  }

  *outfd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (*outfd == -1) {
    perror("shm_open");
    return -1;
  }

  if (ftruncate(*outfd, size) < 0) {
    perror("ftruncate");
    return -1;
  }

  // try mapping to the fixed base so that sel4 kernel can easily use the offset to emulate
  // the physical memory
  void *addr = mmap(base, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, *outfd, 0);
  assert((word_t)addr == (word_t)base);
  if (addr == MAP_FAILED) {
    perror("mmap");
    return -1;
  }

  return 0;
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

int main() {

  /* Install a clenup handler on normal exit */
  atexit(cleanup_onexit);

  int shmfd;
  if (init_sel4emu_pmem(EMUPMEM_FILE, (void *)SEL4_EMU_PMEM_BASE, SEL4_EMU_PMEM_SIZE, &shmfd) < 0) {
    fprintf(stderr, "Failed to initialize emulated physical memory.\n");
    exit(EXIT_FAILURE);
  }

  int connection_socket;
  if (init_uds_socket(SOCKET_NAME, &connection_socket) < 0) {
    fprintf(stderr, "Failed to initialize UNIX Domain Socket.\n");
    unlink(EMUPMEM_FILE);
    exit(EXIT_FAILURE);
  }

  // Initialize the global seL4 threads tracking array
  seL4emu_bk_tid_init();

  // emulate system boot to get the bootinfo and intialize the system
  if (seL4emu_boot() < 0) {
    fprintf(stderr, "Failed to boot seL4 emulator.\n");
  }

  tcb_t *roottcb = NODE_STATE(ksCurThread);
  assert(roottcb != NULL);

  // save the current emulation system state
  seL4emu_g_sysstate.emu_pmem.base = (void *)SEL4_EMU_PMEM_BASE;
  seL4emu_g_sysstate.emu_pmem.size = SEL4_EMU_PMEM_SIZE;
  seL4emu_g_sysstate.emu_pmem.fd = shmfd;
  // TODO(Jiawei): how much pmem left?
  seL4emu_g_sysstate.conn_sock = connection_socket;
  seL4emu_g_sysstate.seL4emu_curthread = roottcb;
  seL4emu_g_sysstate.seL4roottask = roottcb;
  seL4emu_g_sysstate.rt_obj = &rootserver;

  // success on initialization, fork and execute our roottask.
  fprintf(stdout, "\n=======================================================\n");
  fprintf(stdout, "Kernel emulator initialization succeed, now launching the rootask.\n\n");

  pid_t pid = fork();
  if (pid > 0) {
    fprintf(stdout, "Roottask starts as process with id: %d.\n", pid);

    /* At this stage, we al ready set up the roottask tcb if successfull */
    if (seL4emu_bk_tcbptr_insert(pid, roottcb) < 0) {
      fprintf(stderr,
              "Failed to insert a new pid in seL4 thread id list.\n");
      goto exit_on_fail;
    }

  } else if (pid == 0) {
    /* pass the boot info to the roottask */
    if (execve(ROOT_TASK_PATH, NULL, NULL) < 0) {
      perror("execve");
      goto exit_on_fail;
    }
  } else {
    perror("fork");
    goto exit_on_fail;
  }

  syscall_loop(connection_socket);

  // child status
  int status;

  // at this stage all the resources we need have already been created, so clean up all!
exit_on_fail:
  waitpid(-1, &status, 0);
  munmap((void *)SEL4_EMU_PMEM_BASE, SEL4_EMU_PMEM_SIZE);
  close(connection_socket);
  // unlink the socket
  unlink(SOCKET_NAME);
  shm_unlink(EMUPMEM_FILE);
}