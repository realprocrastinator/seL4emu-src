#include "../include/mini_syscalls.h"
#include "../include/mini_string.h"
#include "../include/mini_stdio.h"

_Noreturn void mini_abort(void) {
  // raise(SIGABRT);

	/* If there was a SIGABRT handler installed and it returned, or if
	 * SIGABRT was blocked or ignored, take an AS-safe lock to prevent
	 * sigaction from installing a new SIGABRT handler, uninstall any
	 * handler that may be present, and re-raise the signal to generate
	 * the default action of abnormal termination. */
	
	// __block_all_sigs(0);
	// LOCK(__abort_lock);
	// __syscall(SYS_rt_sigaction, SIGABRT,
	// 	&(struct k_sigaction){.handler = SIG_DFL}, 0, _NSIG/8);
	// __syscall(SYS_tkill, __pthread_self()->tid, SIGABRT);
	// __syscall(SYS_rt_sigprocmask, SIG_UNBLOCK,
	// 	&(long[_NSIG/(8*sizeof(long))]){1UL<<(SIGABRT-1)}, 0, _NSIG/8);

	// /* Beyond this point should be unreachable. */
	// a_crash();
	// raise(SIGKILL);
	// _Exit(127);
	
	// TODO(Jiawei): As the signal hasn't been finished yet, we call exit to simulate the aborting. Make sure we change it later once the signal works.
	_mini_Exit(127);
}

_Noreturn void __mini_assert_fail(const char *expr, const char *file, int line, const char *func)
{
	mini_printf("seL4 emulation assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
	mini_abort();
}