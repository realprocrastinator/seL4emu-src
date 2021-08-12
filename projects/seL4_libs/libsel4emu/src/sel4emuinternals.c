#include <mini_assert.h>
#include <mini_errno.h>
#include <mini_signal.h>
#include <mini_stdio.h>
#include <mini_syscalls.h>
#include <sel4/sel4.h>
#include <sel4emuerror.h>
#include <sel4emuinternals.h>
#include <sel4emuipc.h>
#include <string.h>
#include <sys/mini_mman.h>
#include <sys/mini_socket.h>
#include <sys/un.h>
#include <mini_fcntl.h>

/* global structure to memorize the state after runtime env setup for the emulation framework */
seL4emu_init_state_t seL4emu_g_initstate;

#define EMUPMEM_FILE "/dev/shm/seL4emu_pmem"
#define BIT(n) (1 << n)

static seL4emu_err_t create_and_connect_socket(int *data_socket) {

  /* we are going to use the UNIX domain socket to send the IPC message */
  *data_socket = mini_socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (data_socket < 0) {
    return SEL4EMU_INIT_SOCK_FAILED;
  }

  /* zero out the structure for copying the data */
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  /* try connecting the socket */
  int ret;
  ret = mini_connect(*data_socket, (const struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    return SEL4EMU_INIT_CONNECT_FAILED;
  }

  return SEL4EMU_INIT_NOERROR;
}

static void *try_map(void *vaddr, size_t len, int prot, int flag, int fd, off_t offset) {
  return mini_mmap(vaddr, len, prot, flag, fd, offset);
}

static seL4emu_err_t get_and_map_bootinfo(int socket, seL4_BootInfo **bootinfo) {
  seL4emu_ipc_message_t msg;

  // read socket and block
  if (seL4emu_uds_recv_impl(socket, sizeof(msg), &msg) < 0) {
    return SEL4EMU_INIT_RD_FAILED;
  }

  // must be a internal message from kernel emulator, otherwise this
  // is a bug!
  mini_assert(msg.tag == IPC_INTERNAL);
  mini_assert(msg.words[0] == SEL4EMU_INTERNAL_BOOTINFO);

  // unmarshal the data
  // words[1] is the bootvaddr that kernel decided for us, try map!
  // words[2] is the fd for shared memory
  // words[3] is the fd for mmap offset
  uintptr_t ipcbuf_ptr = msg.words[1];
  int map_fd = msg.words[2];
  size_t offset = msg.words[3];

  map_fd = mini_open(EMUPMEM_FILE, O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (map_fd < 0) {
    return SEL4EMU_INIT_OPEN_FAILED;
  }

  // try map an area for bootinfo, be aware ipcbuffer is below the bootinfo frame!
  void *maybe_new = try_map((void *)ipcbuf_ptr, BIT(seL4_PageBits) * 2, PROT_READ | PROT_WRITE,
                            MAP_SHARED, map_fd, offset);
  if (maybe_new == MAP_FAILED) {
    return SEL4EMU_INIT_BIMMAP_FAILED;
  }

  // update the ptr
  ipcbuf_ptr = (uintptr_t)maybe_new;
  *bootinfo = (seL4_BootInfo *)(maybe_new + BIT(seL4_PageBits));
  (*bootinfo)->ipcBuffer = (seL4_IPCBuffer *)ipcbuf_ptr;

  // DEBUG:
  for (int i = 0; i < 50; i++)
    (*bootinfo)->ipcBuffer->msg[i] = 666;

  // try map a region for ipc buffer
  // maybe_new = try_map((void *)ipcbuf_ptr, BIT(seL4_PageBits), PROT_READ | PROT_WRITE, MAP_SHARED,
  //                     map_fd, offset + bootinfo_ptr);
  // if (maybe_new == MAP_FAILED) {
  //   return SEL4EMU_INIT_IPCBUFMMAP_FAILED;
  // }

  (*bootinfo)->ipcBuffer = (seL4_IPCBuffer *)maybe_new;

  // return on success
  return SEL4EMU_INIT_NOERROR;
}

static void sigsegvhdlr(int sig, siginfo_t *info, void *ctx) {
  if (sig == SIGSEGV) {
    mini_printf("SEGV @%lx!\n", (unsigned long)info->si_addr);
    // check align with one page;
    mini_assert((((unsigned long)info->si_addr) & (0x1000 - 1)) == 0);
    char *addr = (char *)mini_mmap(info->si_addr, 4096, PROT_READ | PROT_WRITE,
                                   MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    if (addr == MAP_FAILED) {
      mini_perror("mini_mmap");
      _exit(0); // halt in a infinite loop
    }
    mini_printf("New mapping addr @%p.\n", addr);
    mini_assert((((unsigned long)info->si_addr) & ~(0x1000 - 1)) == (unsigned long)addr);
  }
}

static seL4emu_err_t initial_sigsegv_handler(void) {
  // setup signal handler for SIGSEGV
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sigset_t mask;
  mini_sigemptyset(&mask);

  sa.sa_flags = SA_SIGINFO | SA_NODEFER;
  sa.sa_sigaction = sigsegvhdlr;
  sa.sa_mask = mask;

  if (mini_sigaction(SIGSEGV, &sa, NULL) < 0) {
    return SEL4EMU_INIT_SIGNAL_FAILED;
  }
}

static seL4emu_err_t greet_kernel_emulator(int socket) {
  seL4emu_err_t err = SEL4EMU_INIT_NOERROR;

  seL4emu_ipc_message_t msg;
  msg.tag = IPC_INTERNAL;
  msg.len = 1;
  msg.words[0] = SEL4EMU_INTERNAL_CLIENT_HELLO;

  // read socket and block
  if (seL4emu_uds_send_impl(socket, sizeof(msg), &msg) < 0) {
    err = SEL4EMU_INIT_WR_FAILED;
  }

  return err;
}

static seL4emu_err_t seL4emu_internal_try_setup(void) {
  seL4emu_err_t err = SEL4EMU_INIT_NOERROR;

  int sock;
  // open socker connect to the kernel emulator
  err = create_and_connect_socket(&sock);
  if (err != SEL4EMU_INIT_NOERROR) {
    return err;
  }
  ST_SET_SOCK(seL4emu_g_initstate, sock);

  // inform kernel emulator we are ready!
  err = greet_kernel_emulator(sock);
  if (err != SEL4EMU_INIT_NOERROR) {
    return err;
  }

  // map the share mem for bootinfo and ipcBuffer, if fail we clean up internally
  seL4_BootInfo *bootinfo_ptr;
  err = get_and_map_bootinfo(sock, &bootinfo_ptr);
  if (err != SEL4EMU_INIT_NOERROR) {
    return err;
  }
  ST_SET_BOOTINFOPTR(seL4emu_g_initstate, bootinfo_ptr);
  ST_SET_IPCBUFPTR(seL4emu_g_initstate, bootinfo_ptr->ipcBuffer);

  // install signal handlers
  err = initial_sigsegv_handler();
  if (err != SEL4EMU_INIT_NOERROR) {
    return err;
  }
  return err;
}

void seL4emu_internal_setup(seL4_BootInfo **bootinfo, seL4_IPCBuffer **ipc_buffer) {
  seL4emu_err_t err;

  seL4emu_initstate();

  /* try do a initial setup */
  err = seL4emu_internal_try_setup();
  bool try_report = true;

  /* resource clean up on error */
  switch (err) {
  case SEL4EMU_INIT_NOERROR:
    *bootinfo = ST_GET_BOOTINFOPTR(seL4emu_g_initstate);
    *ipc_buffer = ST_GET_IPCBUFPTR(seL4emu_g_initstate);
    break;
  case SEL4EMU_INIT_SIGNAL_FAILED:
    mini_munmap(ST_GET_IPCBUFPTR(seL4emu_g_initstate), BIT(seL4_PageBits));
    mini_munmap(ST_GET_BOOTINFOPTR(seL4emu_g_initstate), BIT(seL4_PageBits));
    ST_SET_BOOTINFOPTR(seL4emu_g_initstate, NULL);
    ST_SET_IPCBUFPTR(seL4emu_g_initstate, NULL);
    break;
  case SEL4EMU_INIT_IPCBUFMMAP_FAILED:
    mini_munmap(ST_GET_BOOTINFOPTR(seL4emu_g_initstate), BIT(seL4_PageBits));
    ST_SET_BOOTINFOPTR(seL4emu_g_initstate, NULL);
    break;
  case SEL4EMU_INIT_BIMMAP_FAILED:
    break;
  case SEL4EMU_INIT_OPEN_FAILED:
    break;
  case SEL4EMU_INIT_CONNECT_FAILED:
    close(ST_GET_SOCK(seL4emu_g_initstate));
    ST_SET_SOCK(seL4emu_g_initstate, -1);
  case SEL4EMU_INIT_SOCK_FAILED:
    try_report = false;
    break;
  default:
    mini_assert(!"Shouldn't reach here!");
  }

  if (err) {
    mini_perror("Emulation Internal Env Initial Fialed");
  }

  if (try_report) {
    seL4emu_ipc_message_t msg;
    msg.tag = IPC_INTERNAL;

    // first word is the state
    int state =
        err == SEL4EMU_INIT_NOERROR ? SEL4EMU_INTERNAL_INIT_SUCCESS : SEL4EMU_INTERNAL_INIT_FAILED;
    msg.words[0] = state;

    // report to server
    int sock = ST_GET_SOCK(seL4emu_g_initstate);
    if (seL4emu_uds_send_impl(sock, sizeof(msg), &msg) < 0) {
      err = SEL4EMU_INIT_WR_FAILED;
    }

    if (err != SEL4EMU_INIT_NOERROR) {
      close(sock);
    }
  }

  ST_SET_EMUERR(seL4emu_g_initstate, err);
  ST_SET_SYSERR(seL4emu_g_initstate, mini_errno);

  /* If we can't initial successfully, there is no point to continue*/
  mini_assert(err == SEL4EMU_INIT_NOERROR);
  mini_assert(mini_errno == 0);

  return;
}