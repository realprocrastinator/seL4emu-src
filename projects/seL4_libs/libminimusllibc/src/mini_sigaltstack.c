#define _GNU_SOURCE
#include "../include/mini_signal.h"
#include "../include/mini_errno.h"
#include "mini_internal_syscall.h"

int mini_sigaltstack(const stack_t *restrict ss, stack_t *restrict old)
{
	if (ss) {
		if (ss->ss_size < MINSIGSTKSZ) {
			mini_errno = ENOMEM;
			return -1;
		}
		if (ss->ss_flags & ~SS_DISABLE) {
			mini_errno = EINVAL;
			return -1;
		}
	}
	return mini_syscall(SYS_sigaltstack, ss, old);
}
