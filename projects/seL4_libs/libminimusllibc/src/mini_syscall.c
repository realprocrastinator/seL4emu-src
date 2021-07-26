#include "../include/mini_unistd.h"
#include "mini_internal_syscall.h"
#include "../include/mini_stdarg.h"

/* there was a macro defined in mini_internal_syscall for internal usage */
#undef mini_syscall

long mini_syscall(long n, ...)
{
	va_list ap;
	syscall_arg_t a,b,c,d,e,f;
	va_start(ap, n);
	a=va_arg(ap, syscall_arg_t);
	b=va_arg(ap, syscall_arg_t);
	c=va_arg(ap, syscall_arg_t);
	d=va_arg(ap, syscall_arg_t);
	e=va_arg(ap, syscall_arg_t);
	f=va_arg(ap, syscall_arg_t);
	va_end(ap);
	return __mini_syscall_ret(__mini_syscall(n,a,b,c,d,e,f));
}