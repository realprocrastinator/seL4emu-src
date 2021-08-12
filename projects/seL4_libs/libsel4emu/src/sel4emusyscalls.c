#include <mini_assert.h>
#include <mini_stdio.h>
#include <mini_syscalls.h>
#include <sel4/types.h>
#include <sel4emuipc.h>
#include <sel4emusyscalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOT_IMPLEMENTED (!"Not implemented yet")

void seL4emu_sys_send(seL4_Word sys, seL4_Word dest, seL4_Word info,
                      seL4_Word msg0, seL4_Word msg1, seL4_Word msg2,
                      seL4_Word msg3) {

  /* construct the seL4 ipc message */
  seL4emu_ipc_message_t msg;
  msg.tag = IPC_SEL4;
  /* 7 registers to be passed */
  msg.len = 7;
  msg.id = mini_getpid();
  /* syscall number */
  seL4emu_set_ipc_register(sys, &msg, RDX);
  /* cap register */
  seL4emu_set_ipc_register(dest, &msg, RDI);
  /* msg info register */
  seL4emu_set_ipc_register(info, &msg, RSI);
  /* messgae register */
  seL4emu_set_ipc_register(msg0, &msg, R10);
  /* messgae register */
  seL4emu_set_ipc_register(msg1, &msg, R8);
  /* messgae register */
  seL4emu_set_ipc_register(msg2, &msg, R9);
  /* messgae register */
  seL4emu_set_ipc_register(msg3, &msg, R15);

  /* send ipc using UDS */
  seL4emu_uds_send(&msg, sizeof(msg));
}

void seL4emu_sys_reply(seL4_Word sys, seL4_Word info, seL4_Word msg0,
                       seL4_Word msg1, seL4_Word msg2, seL4_Word msg3) {
  mini_assert(NOT_IMPLEMENTED);
}

void seL4emu_sys_send_null(seL4_Word sys, seL4_Word dest, seL4_Word info) {
  switch (sys) {
  case seL4_SysSetTLSBase: {
    /**
     * We check if we can use the fsgsbase instruction faimily, if yes then we
     * can setup the TLS in the user space, other wise ask host kernel to do it.
     * TODO(Jiawei): As the kernel emulator are not fully functional, we do it
     * ourselves, should ask kernel emulator to do validation later!
     */

    /* TODO(Jiawei): query the CPUID to check if can use fabase instruction
     * familly */
    // mini_printf("Got syscall seL4_SysSetTLSBase, setting up the TLS now.\n");
    unsigned long base = 0;

    // asm volatile("rdfsbase %0" : "=r"(base));
    // mini_printf("Old seL4_SysSetTLSBase is at: %lx now.\n", base);

    asm volatile("wrfsbase %0" ::"r"(dest));

    // asm volatile("rdfsbase %0" : "=r"(base));
    // mini_printf("New seL4_SysSetTLSBase is at: %lx now.\n", base);

    break;
  }
  default:
    /* TODO(Jiawei): Other syscalls requests we will just forward to the kernel
     * emulator */
    // mini_printf("Got syscalls other then seL4_SysSetTLSBase, forwarding to the "
    //             "server now.\n");
    break;
  }
}

void seL4emu_sys_recv(seL4_Word sys, seL4_Word src, seL4_Word *out_badge,
                      seL4_Word *out_info, seL4_Word *out_mr0,
                      seL4_Word *out_mr1, seL4_Word *out_mr2,
                      seL4_Word *out_mr3, LIBSEL4_UNUSED seL4_Word reply) {
  mini_assert(NOT_IMPLEMENTED);
}

void seL4emu_sys_send_recv(seL4_Word sys, seL4_Word dest, seL4_Word *out_dest,
                           seL4_Word info, seL4_Word *out_info,
                           seL4_Word *in_out_mr0, seL4_Word *in_out_mr1,
                           seL4_Word *in_out_mr2, seL4_Word *in_out_mr3,
                           LIBSEL4_UNUSED seL4_Word reply) {

  if (sys == seL4_SysDebugPutChar) {
    // TODO(Jiawei): Currently just print out whatevert the char is using local
    // print functions instead of sending to the kernel emulator.
    mini_write(1, &dest, 1);

  } else {
    /* construct the seL4 ipc message */
    seL4emu_ipc_message_t msg;
    msg.tag = IPC_SEL4;
    msg.len = 7;
    msg.id = mini_getpid();
    /* syscall number */
    seL4emu_set_ipc_register(sys, &msg, RDX);
    /* cap register */
    seL4emu_set_ipc_register(dest, &msg, RDI);
    /* msg info register */
    seL4emu_set_ipc_register(info, &msg, RSI);
    /* messgae register */
    seL4emu_set_ipc_register(*in_out_mr0, &msg, R10);
    /* messgae register */
    seL4emu_set_ipc_register(*in_out_mr1, &msg, R8);
    /* messgae register */
    seL4emu_set_ipc_register(*in_out_mr2, &msg, R9);
    /* messgae register */
    seL4emu_set_ipc_register(*in_out_mr3, &msg, R15);

    /* send ipc using UDS */
    seL4emu_uds_send(&msg, sizeof(msg));


    /* This step we emulate the returning from the syscall */
    /*recv result*/
    seL4emu_uds_recv(&msg, sizeof(msg));

    *out_info = seL4emu_get_ipc_register(&msg, RSI);
    *out_dest = seL4emu_get_ipc_register(&msg, RDI);
    *in_out_mr0 = seL4emu_get_ipc_register(&msg, R10);
    *in_out_mr1 = seL4emu_get_ipc_register(&msg, R8);
    *in_out_mr2 = seL4emu_get_ipc_register(&msg, R9);
    *in_out_mr3 = seL4emu_get_ipc_register(&msg, R15);
  }
}

void seL4emu_sys_nbsend_recv(seL4_Word sys, seL4_Word dest, seL4_Word src,
                             seL4_Word *out_dest, seL4_Word info,
                             seL4_Word *out_info, seL4_Word *in_out_mr0,
                             seL4_Word *in_out_mr1, seL4_Word *in_out_mr2,
                             seL4_Word *in_out_mr3, seL4_Word reply) {
  mini_assert(NOT_IMPLEMENTED);
}

void seL4emu_sys_null(seL4_Word sys) { mini_assert(NOT_IMPLEMENTED); }

// void seL4emu_DebugPutChar(char c) {
//   struct sockaddr_un addr;
//   int ret;
//   int data_socket;
//   char buffer[BUFFER_SIZE];

//   data_socket = mini_socket(AF_UNIX, SOCK_SEQPACKET, 0);
//   if (data_socket < 0) {
//     mini_perror("Failed to create the uds socket");
//     return;
//   }

//   memset(&addr, 0, sizeof(addr));

//   addr.sun_family = AF_UNIX;
//   strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

//   ret = mini_connect(data_socket, (const struct sockaddr*) &addr,
//   sizeof(addr)); if (ret < 0 ) {
//     mini_perror("Failed to connect the uds socket");
//     return;
//   }

//   ret = mini_write(data_socket, &c, 2);
//   if (ret < 0) {
//     mini_perror("Failed to write to the uds socket");
//     return;
//   }

//   // receive result
//   ret = mini_read(data_socket, buffer, 2);
//   if (ret < 0) {
//     mini_perror("Failed to read from the uds socket");
//     return;
//   }

//   // ensure the buffer is null terminated
//   buffer[BUFFER_SIZE - 1] = 0;

//   ret = mini_write(STDOUT_FILENO, buffer, BUFFER_SIZE);
//   if (ret < 0) {
//     mini_perror("Failed to write to the stdout");
//     return;
//   }

//   // close the socket
//   mini_close(data_socket);
// }
