#include <mini_assert.h>
#include <mini_errno.h>
#include <mini_stdio.h>
#include <mini_syscalls.h>
#include <sel4emuinternals.h>
#include <sel4emuipc.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

static int seL4emu_rw_all(int fd, bool is_read, size_t len, void *buff) {
  mini_assert(buff);
  unsigned long cbuff = (unsigned long)buff;

  while (len) {
    ssize_t rd;
    if (is_read) {
      rd = mini_read(fd, (void *)cbuff, len);
    } else {
      rd = mini_write(fd, (void *)cbuff, len);
    }

    if (rd == 0) {
      // EOF
      // Requested IO operation ended prematurely.
      mini_printf("Emulation internal Requested IO operation ended prematurely.\n");
      return -1;
    } else if (rd < 0) {
      switch (mini_errno) {
      case EINTR:
      case EWOULDBLOCK:
        // retry
        continue;
      }
      // an actual error!
      // IO error
      mini_perror("Emulation internal mini_rw_all IO error");
      return -1;
    } else {
      // as usual, we assume rd never larger than len
      len -= rd;
      cbuff += rd;
    }
  }

  return 0;
}

int seL4emu_uds_send_impl(int fd, size_t len, void *msg) {
  int ret = 0;

  /* write the data to the socket */
  ret = seL4emu_rw_all(fd, false, len, msg);

  return ret;
}

int seL4emu_uds_recv_impl(int fd, size_t len, void *msg) {
  int ret = 0;

  /* read the data to the socket */
  ret = seL4emu_rw_all(fd, true, len, msg);

  return ret;
}

int seL4emu_uds_send(void *msg, size_t len) {
  mini_assert(msg);
  int fd = ST_GET_SOCK(seL4emu_g_initstate);
  mini_assert(fd > 0);

  seL4emu_uds_send_impl(fd, len, msg);
}

int seL4emu_uds_recv(void *msg, size_t len) {
  mini_assert(msg);
  int fd = ST_GET_SOCK(seL4emu_g_initstate);
  mini_assert(fd > 0);

  seL4emu_uds_recv_impl(fd, len, msg);
}
