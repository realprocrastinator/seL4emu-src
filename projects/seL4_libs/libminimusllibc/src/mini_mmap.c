#include "../include/mini_unistd.h"
#include "../include/sys/mini_mman.h"
#include "../include/mini_errno.h"
#include "../include/mini_stdint.h"
#include "../include/mini_limits.h"
#include "mini_internal_syscall.h"
// #include "libc.h"

// static void dummy(void) { }
// weak_alias(dummy, __vm_wait);

#define UNIT SYSCALL_MMAP2_UNIT
#define OFF_MASK ((-0x2000ULL << (8*sizeof(long)-1)) | (UNIT-1))

void *__mini_mmap(void *start, size_t len, int prot, int flags, int fd, off_t off)
{
	if (off & OFF_MASK) {
		mini_errno = EINVAL;
		return MAP_FAILED;
	}
	if (len >= PTRDIFF_MAX) {
		mini_errno = ENOMEM;
		return MAP_FAILED;
	}
	if (flags & MAP_FIXED) {
		// __vm_wait();
	}
#ifdef SYS_mmap2
	return (void *)mini_syscall(SYS_mmap2, start, len, prot, flags, fd, off/UNIT);
#else
	return (void *)mini_syscall(SYS_mmap, start, len, prot, flags, fd, off);
#endif
}

weak_alias(__mini_mmap, mini_mmap);

LFS64(mini_mmap);
