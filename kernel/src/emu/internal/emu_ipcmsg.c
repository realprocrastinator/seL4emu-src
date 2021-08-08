#define _GNU_SOURCE
#include <emu/emu_common.h>
#include <emu/emu_ipcmsg.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int rw_all(int fd, bool is_read, size_t len, void *buff);

int seL4emu_uds_send(int fd, size_t len, void *msg) {
  int ret = 0;

  /* write the data to the socket */
  ret = rw_all(fd, false, len, msg);

  return ret;
}

int seL4emu_uds_recv(int fd, size_t len, void *msg) {
  int ret = 0;

  /* read the data to the socket */
  ret = rw_all(fd, true, len, msg);

  return ret;
}

static int rw_all(int fd, bool is_read, size_t len, void *buff) {
  assert(buff);
  unsigned long cbuff = (unsigned long)buff;

  while (len) {
    ssize_t rd;
    if (is_read) {
      rd = read(fd, (void *)cbuff, len);
    } else {
      rd = write(fd, (void *)cbuff, len);
    }

    if (rd == 0) {
      // EOF
      // Requested IO operation ended prematurely.
      fprintf(stderr, "Emulation internal Requested IO operation ended prematurely.\n");
      return -1;
    } else if (rd < 0) {
      switch (errno) {
      case EINTR:
      case EWOULDBLOCK:
        // retry
        continue;
      }
      // an actual error!
      // IO error
      perror("IO error");
      return -1;
    } else {
      // as usual, we assume rd never larger than len
      len -= rd;
      cbuff += rd;
    }
  }

  return 0;
}