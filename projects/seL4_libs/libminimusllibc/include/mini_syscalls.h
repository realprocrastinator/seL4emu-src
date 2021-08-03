#include "mini_unistd.h"
#include "sys/mini_socket.h"

/* I/O operations */
ssize_t mini_read(int fd, void *buf, size_t count);
ssize_t mini_write(int fd, const void *buf, size_t count);
int mini_close(int fd);
int mini_open(const char *filename, int flags, ...);
int mini_unlink(const char *path);

/* exit */
_Noreturn void _mini_Exit(int ec);

/* error printing */
void mini_perror(const char* msg);

/* socket operations */
int mini_socket (int, int, int);
int mini_bind (int, const struct sockaddr *, socklen_t);
int mini_connect (int, const struct sockaddr *, socklen_t);
int mini_listen (int, int);
int mini_accept (int, struct sockaddr *__restrict, socklen_t *__restrict);

/* process id */
pid_t mini_getpid(void);

/* memory */


long mini_syscall(long, ...);