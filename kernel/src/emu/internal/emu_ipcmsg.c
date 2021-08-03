#define _GNU_SOURCE
#include <emu/emu_ipcmsg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

// TODO(Jiawei): these macro only for debugging usage, should be moved to an appropriate place later.
#define SOCKET_NAME "/tmp/uds-test.socket"

int seL4emu_ipc_send(seL4emu_ipc_message_t *msg, size_t len) {
  struct sockaddr_un addr;
  int ret;
  int data_socket;

  /* we are going to use the UNIX domain socket to send the IPC message */
  data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (socket < 0) {
    perror("Emulation internal: mini_socket");
    goto send_fail;
  }

  /* zero out the structure for copying the data */
  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  /* try connecting the socket */
  ret = connect(data_socket, (const struct sockaddr*) &addr, sizeof(addr));
  if (ret < 0 ) {
    perror("Emulation internal: mini_connect");
    goto send_fail;
  }

  /* write the data to the socket */
  ret = write(data_socket, msg, len);
  if (ret < 0) {
    perror("Emulation internal: mini_write");
    goto send_fail;
  }

  /* return bytes sent on success */
  return ret;

send_fail:
  // close the socket
  close(data_socket);
  return -1;
}

int seL4emu_ipc_recv(seL4emu_ipc_message_t *msg, size_t len) {
  struct sockaddr_un addr;
  int ret;
  int data_socket;

  /* we are going to use the UNIX domain socket to send the IPC message */
  data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (data_socket < 0) {
    perror("Emulation internal: mini_socket");
    goto recv_fail;
  }

  /* zero out the structure for copying the data */
  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  /* try connecting the socket */
  ret = connect(data_socket, (const struct sockaddr*) &addr, sizeof(addr));
  if (ret < 0 ) {
    perror("Emulation internal: connect");
    goto recv_fail;
  }

  // receive result
  ret = read(data_socket, msg, len);
  if (ret < 0) {
    perror("Emulation internal: read");
    goto recv_fail;
  }

  // on success we return bytes read
  return ret;

recv_fail:
  close(data_socket);
  return -1;
}