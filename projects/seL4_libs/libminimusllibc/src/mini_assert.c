#include "../include/mini_syscalls.h"
#include "../include/mini_string.h"
#include "../include/mini_stdio.h"

_Noreturn void abort(void) {
  _mini_Exit(127);
}

_Noreturn void __mini_assert_fail(const char *expr, const char *file, int line, const char *func)
{
	mini_printf("seL4 emulation assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
	abort();
}