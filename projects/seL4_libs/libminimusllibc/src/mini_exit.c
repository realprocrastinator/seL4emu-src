#include "mini_internal_syscall.h"

_Noreturn void _mini_Exit(int ec)
{
	__mini_syscall(SYS_exit_group, ec);
	for (;;) __mini_syscall(SYS_exit, ec);
}