#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mini_syscalls.h>

#define SOCKET_NAME "/tmp/uds-test.socket"
#define BUFFER_SIZE 12

void seL4emu_DebugPutChar(char c) {
  struct sockaddr_un addr;
  int ret;
  int data_socket;
  char buffer[BUFFER_SIZE];

  data_socket = mini_socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (data_socket < 0) {
    // TODO: error report
    return;
  }

  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  ret = mini_connect(data_socket, (const struct sockaddr*) &addr, sizeof(addr));
  if (ret < 0 ) {
    // TODO: error report
    return;
  }

  ret = mini_write(data_socket, &c, 2);
  if (ret < 0) {
    // TODO: error report
    return;
  }

    // receive result
  ret = mini_read(data_socket, buffer, 2);
  if (ret == -1) {
    // TODO: error report
    return;
  }

  // ensure the buffer is null terminated
  buffer[BUFFER_SIZE - 1] = 0;

  ret = mini_write(1, buffer, 2);
  if (ret < 0) {
    // TODO: error report
    return;
  }

  // close the socket
  mini_close(data_socket);
}