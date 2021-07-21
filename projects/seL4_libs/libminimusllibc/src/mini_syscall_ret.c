#include "../include/mini_errno.h"
#include "mini_internal_syscall.h"

extern int mini_errno;

long __mini_syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		mini_errno = -r;
		return -1;
	}
	return r;
}