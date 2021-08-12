#include "../include/mini_signal.h"
#include "mini_internal_syscall.h"
// #include "pthread_impl.h"

void __mini_block_app_sigs(void *set);
void __mini_restore_sigs(void *set);

int mini_raise(int sig)
{
	int tid, ret;
	sigset_t set;
	__mini_block_app_sigs(&set);
	tid = __mini_syscall(SYS_gettid);
	ret = mini_syscall(SYS_tkill, tid, sig);
	__mini_restore_sigs(&set);
	return ret;
}
