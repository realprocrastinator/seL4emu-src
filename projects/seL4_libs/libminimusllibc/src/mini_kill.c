#include "../include/mini_signal.h"
#include "mini_internal_syscall.h"

int mini_kill(pid_t pid, int sig)
{
	return mini_syscall(SYS_kill, pid, sig);
}