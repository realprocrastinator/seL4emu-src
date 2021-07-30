#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>

// sel4
#include <sel4/shared_types_gen.h>
// #include <autoconf.h>

#define SOCKET_NAME "/tmp/uds-test.socket"
#define BUFFER_SIZE (27 * 8)

enum _register {
    // User registers that will be preserved during syscall
    // Deliberately place the cap and badge registers early
    // So that when popping on the fastpath we can just not
    // pop these
    RDI                     = 0,    /* 0x00 */
    capRegister             = 0,
    badgeRegister           = 0,
    RSI                     = 1,    /* 0x08 */
    msgInfoRegister         = 1,
    RAX                     = 2,    /* 0x10 */
    RBX                     = 3,    /* 0x18 */
    RBP                     = 4,    /* 0x20 */
    R12                     = 5,    /* 0x28 */
#ifdef CONFIG_KERNEL_MCS
    replyRegister           = 5,
#endif
    R13                     = 6,    /* 0x30 */
#ifdef CONFIG_KERNEL_MCS
    nbsendRecvDest          = 6,
#endif
    R14                     = 7,    /* 0x38 */
    RDX                     = 8,    /* 0x40 */
    // Group the message registers so they can be efficiently copied
    R10                     = 9,    /* 0x48 */
    R8                      = 10,   /* 0x50 */
    R9                      = 11,   /* 0x58 */
    R15                     = 12,   /* 0x60 */
    FLAGS                   = 13,   /* 0x68 */
    // Put the NextIP, which is a virtual register, here as we
    // need to set this in the syscall path
    NextIP                  = 14,   /* 0x70 */
    // Same for the error code
    Error                   = 15,   /* 0x78 */
    /* Kernel stack points here on kernel entry */
    RSP                     = 16,   /* 0x80 */
    FaultIP                 = 17,   /* 0x88 */
    // Now user Registers that get clobbered by syscall
    R11                     = 18,   /* 0x90 */
    RCX                     = 19,   /* 0x98 */
    CS                      = 20,   /* 0xa0 */
    SS                      = 21,   /* 0xa8 */
    n_immContextRegisters   = 22,   /* 0xb0 */

    // For locality put these here as well
    FS_BASE                 = 22,   /* 0xb0 */
    TLS_BASE                = FS_BASE,
    GS_BASE                 = 23,   /* 0xb8 */

    n_contextRegisters      = 24    /* 0xc0 */
};

enum seL4emu_ipc_tag {
    ipc_sel4 = 0,
    ipc_internal  = 1
};

/**
 * This the message structure for emulating the seL4 IPC, the mesassage has several types, using tag 
 * to distinguish them, if the mesassage uses the seL4_sys_emu tag, then the content emulates the registers
 * when the real seL4 syscall call happen. The registers order follows the syscall calling convention and is
 * architectual dependent. Here we use the x86_64 calling convention.   
 */

struct seL4emu_ipc_message {
  /**
   * tag specifies whether this IPC messgae is for the internal usage of
   * the emulation framework, or for emulating the seL4 IPC
   */
  uint64_t tag;
  /* How many words needs to be passed */
  uint64_t len;
  /**
   * We follow the layout of registers defined in the kernel/include/arch/x86/arch/64/mode/machine/registerset.h.
   * Despite that we only use at most 7 registers, but we reserve spaces for all 25 registers. 
   */
  uint64_t words[25];
};

typedef struct seL4emu_ipc_message seL4emu_ipc_message_t;

/* This program is only used for testing whether the build system works */

int main () {
  struct sockaddr_un name;
  int ret;
  int connection_socket;
  int data_socket;
  seL4emu_ipc_message_t buffer;

  // create local socket
  connection_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  
  if (connection_socket == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // clean the whole structure
  memset(&name, 0, sizeof(name));

  // bind socket to the socket name
  name.sun_family = AF_UNIX;
  strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

  ret = bind(connection_socket, (const struct sockaddr *)&name, sizeof(name));
  if (ret == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  // prepare for acceptin connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.

  ret = listen(connection_socket, 20);
  if (ret == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // this is the main loop for handling the connections
  for (;;) {
    // wait for incoming connections
    data_socket = accept(connection_socket, NULL, NULL);
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

    printf("tag=%lu, len=%lu, sys=%ld, dest/capreg=%lu, msginfo=%lu, msg0=%lu, msg1=%lu, msg2=%lu, msg3=%lu\n", buffer.tag, buffer.len, (long)buffer.words[RDX], buffer.words[RDI], buffer.words[RSI], buffer.words[R10], buffer.words[R8], buffer.words[R9], buffer.words[R15]);

    // close the socket
    close(data_socket);

    // check if this is the end
  }

  close(connection_socket);
  puts("Server closed everything.");

  // unlink the socket
  unlink(SOCKET_NAME);
  exit(EXIT_SUCCESS);
}