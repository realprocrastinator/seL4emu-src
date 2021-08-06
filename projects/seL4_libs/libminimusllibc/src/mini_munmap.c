#include "../include/sys/mini_mman.h"
#include "mini_internal_syscall.h"
// #include "libc.h"

// static void dummy(void) { }
// weak_alias(dummy, __vm_wait);

int __mini_munmap(void *start, size_t len)
{
	// __vm_wait();
	return mini_syscall(SYS_munmap, start, len);
}

weak_alias(__mini_munmap, mini_munmap);
