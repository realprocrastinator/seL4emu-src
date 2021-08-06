	nop
.global __mini_restore_rt
.type __mini_restore_rt,@function
__mini_restore_rt:
	mov $15, %rax
	syscall
.size __mini_restore_rt,.-__mini_restore_rt
