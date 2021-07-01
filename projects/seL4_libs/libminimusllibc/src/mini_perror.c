#include "mini_errno.h"
#include "../include/mini_syscalls.h"
#include "../include/mini_string.h"

#ifndef STDERR
#define STDERR 2
#endif

#define BUFSIZE 1024

void mini_perror(const char *msg)
{
	char *errstr = mini_strerror(mini_errno);
  char c;
	
	if (msg && *msg) {
		mini_write(STDERR, msg, mini_strlen(msg));
    c = ':';
    mini_write(STDERR, &c, 1);
    c = ' '; 
		mini_write(STDERR, &c, 1);
	}

  mini_write(STDERR, errstr, mini_strlen(errstr));
	c = '\n';
  mini_write(STDERR, &c, 1);
}
