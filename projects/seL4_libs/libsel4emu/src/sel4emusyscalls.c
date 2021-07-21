#include <sel4/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mini_stdio.h>
#include <mini_syscalls.h>
#include <mini_assert.h>
#include <sel4emusyscalls.h>

// TODO: these macro only for debugging usage, should be moved to an appropriate place later.
#define SOCKET_NAME "/tmp/uds-test.socket"
#define BUFFER_SIZE 2
#define NOT_IMPLEMENTED (!"Not implemented yet")

void seL4emu_sys_send(seL4_Word sys, seL4_Word dest, seL4_Word info, 
                      seL4_Word msg0, seL4_Word msg1, seL4_Word msg2, seL4_Word msg3) {
  // TODO: seL4 send emulation
  mini_assert(NOT_IMPLEMENTED);
}

void seL4emu_sys_reply(seL4_Word sys, seL4_Word info, seL4_Word msg0, 
                       seL4_Word msg1, seL4_Word msg2, seL4_Word msg3) {
  mini_assert(NOT_IMPLEMENTED);                           
}

void seL4emu_sys_send_null(seL4_Word sys, seL4_Word dest, seL4_Word info) {
  // mini_assert(NOT_IMPLEMENTED);
  switch(sys){
  case seL4_SysSetTLSBase: {
    /**
     * We check if we can use the fsgsbase instruction faimily, if yes then we can setup the 
     * TLS in the user space, other wise ask host kernel to do it.
     * TODO: As the kernel emulator are not fully functional, we do it ourselves, should
     * ask kernel emulator to do validation later!   
     */
    
    /* TODO: query the CPUID to check if can use fabase instruction familly */
    mini_printf("Got syscall seL4_SysSetTLSBase, setting up the TLS now.\n");
    unsigned long base = 0;
    asm volatile("rdfsbase %0":"=r"(base));
    mini_printf("Old seL4_SysSetTLSBase is at: %lx now.\n", base);
    
    asm volatile("wrfsbase %0"::"r"(dest));

    asm volatile("rdfsbase %0":"=r"(base));
    mini_printf("New seL4_SysSetTLSBase is at: %lx now.\n", base);

    break;
  }
  default:
    /* TODO: Other syscalls requests we will just forward to the kernel emulator */
    mini_printf("Got syscalls other then seL4_SysSetTLSBase, forwarding to the server now.\n");
    break;
  }
}

void seL4emu_sys_recv(seL4_Word sys, seL4_Word src, seL4_Word *out_badge, seL4_Word *out_info,
                      seL4_Word *out_mr0, seL4_Word *out_mr1, seL4_Word *out_mr2, seL4_Word *out_mr3,
                      LIBSEL4_UNUSED seL4_Word reply) {
  mini_assert(NOT_IMPLEMENTED);
}

void seL4emu_sys_send_recv(seL4_Word sys, seL4_Word dest, seL4_Word *out_dest, seL4_Word info,
                           seL4_Word *out_info, seL4_Word *in_out_mr0, seL4_Word *in_out_mr1, seL4_Word *in_out_mr2, 
                           seL4_Word *in_out_mr3, LIBSEL4_UNUSED seL4_Word reply) {
  
  if (sys == seL4_SysDebugPutChar) {
    // TODO: Currently just print out whatevert the char is using local print functions instead of sending to the kernel emulator.
    mini_write(1, &dest, 1);

  } else {
    // for other syscalls we haven't implemented yet.
    mini_assert(NOT_IMPLEMENTED);  
  }
  
}

void seL4emu_sys_nbsend_recv(seL4_Word sys, seL4_Word dest, seL4_Word src, seL4_Word *out_dest,
                             seL4_Word info, seL4_Word *out_info, seL4_Word *in_out_mr0, seL4_Word *in_out_mr1, 
                             seL4_Word *in_out_mr2, seL4_Word *in_out_mr3, seL4_Word reply) {
  mini_assert(NOT_IMPLEMENTED);                               
}

void seL4emu_sys_null(seL4_Word sys) {
  mini_assert(NOT_IMPLEMENTED);
}

void seL4emu_DebugPutChar(char c) {
  struct sockaddr_un addr;
  int ret;
  int data_socket;
  char buffer[BUFFER_SIZE];

  data_socket = mini_socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (data_socket < 0) {
    mini_perror("Failed to create the uds socket");
    return;
  }

  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  ret = mini_connect(data_socket, (const struct sockaddr*) &addr, sizeof(addr));
  if (ret < 0 ) {
    mini_perror("Failed to connect the uds socket");
    return;
  }

  ret = mini_write(data_socket, &c, 2);
  if (ret < 0) {
    mini_perror("Failed to write to the uds socket");
    return;
  }

  // receive result
  ret = mini_read(data_socket, buffer, 2);
  if (ret < 0) {
    mini_perror("Failed to read from the uds socket");
    return;
  }

  // ensure the buffer is null terminated
  buffer[BUFFER_SIZE - 1] = 0;

  ret = mini_write(STDOUT_FILENO, buffer, BUFFER_SIZE);
  if (ret < 0) {
    mini_perror("Failed to write to the stdout");
    return;
  }

  // close the socket
  mini_close(data_socket);
}



