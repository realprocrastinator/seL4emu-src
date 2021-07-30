#include <sel4emuipc.h>
#include <mini_syscalls.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>


// TODO(Jiawei): these macro only for debugging usage, should be moved to an appropriate place later.
#define SOCKET_NAME "/tmp/uds-test.socket"

void seL4emu_uds_send(seL4emu_ipc_message_t *msg, size_t len) {
  struct sockaddr_un addr;
  int ret;
  int data_socket;

  /* we are going to use the UNIX domain socket to send the IPC message */
  data_socket = mini_socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (data_socket < 0) {
    mini_perror("Emulation internal mini_socket");
    return;
  }

  /* zero out the structure for copying the data */
  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  /* try connecting the socket */
  ret = mini_connect(data_socket, (const struct sockaddr*) &addr, sizeof(addr));
  if (ret < 0 ) {
    mini_perror("Emulation internal mini_connect");
    return;
  }

  /* write the data to the socket */
  ret = mini_write(data_socket, msg, len);
  if (ret < 0) {
    mini_perror("Emulation internal mini_write");
    return;
  }

  // close the socket
  mini_close(data_socket);
}

void seL4emu_uds_recv(seL4emu_ipc_message_t *msg, size_t len) {
  struct sockaddr_un addr;
  int ret;
  int data_socket;

  /* we are going to use the UNIX domain socket to send the IPC message */
  data_socket = mini_socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (data_socket < 0) {
    mini_perror("Emulation internal mini_socket");
    return;
  }

  /* zero out the structure for copying the data */
  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  /* try connecting the socket */
  ret = mini_connect(data_socket, (const struct sockaddr*) &addr, sizeof(addr));
  if (ret < 0 ) {
    mini_perror("Emulation internal mini_connect");
    return;
  }

  // receive result
  ret = mini_read(data_socket, msg, len);
  if (ret < 0) {
    mini_perror("Emulation internal mini_read");
    return;
  }

  // close the socket
  mini_close(data_socket);
}