#include <autoconf.h>
#include <stdio.h>
#include <unistd.h>

#ifdef CONFIG_SEL4_USE_EMULATION
#define _GNU_SOURCE
#include <mini_signal.h>
#include <sys/mini_mman.h>
#include <mini_assert.h>
#include <mini_syscalls.h>
#include <string.h>

static char signal_stack[SIGSTKSZ];

static void sigsegvhdlr(int sig, siginfo_t* info, void* ctx) {
    ucontext_t *uc = (ucontext_t *)ctx;
    greg_t *rip = &uc->uc_mcontext.gregs[REG_RIP];
    // asm volatile (
    //     "movabs $checkpoint, %0"
    //     : "=r" (*rip)
    // );

    if (sig == SIGSEGV) {
        printf("SEGV @%lx!\n", (unsigned long)info->si_addr);
        // check align with one page;
        mini_assert((((unsigned long)info->si_addr) & (0x1000 - 1)) == 0);
        char *addr = (char *)mini_mmap(info->si_addr, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
        if (addr == MAP_FAILED) {
            mini_perror("mini_mmap");
            _exit(0); // halt in a infinite loop
        }
        printf("New mapping addr @%p.\n", addr);
        mini_assert((((unsigned long) info->si_addr) & ~(0x1000 - 1)) == (unsigned long)addr);

    } else {
        printf("SIGNAL: %d\n", sig);
    }
}
#endif

int main(int argc, char *argv[]) {

#if CONFIG_SEL4_USE_EMULATION

    // set up the signal stack
    stack_t ss;
    ss.ss_sp = signal_stack;
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;
    if (mini_sigaltstack(&ss, NULL) == -1) {
               perror("sigaltstack");
               _exit(1);
           }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sigset_t mask;
	mini_sigemptyset(&mask);
    
    sa.sa_flags = SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
    sa.sa_sigaction = sigsegvhdlr;
    sa.sa_mask = mask;

    mini_sigaction(SIGSEGV, &sa, NULL);
#endif

    printf("Hello, World!\n");


    char * a = (char *)0xffffff80000;
    // asm volatile ("checkpoint: .global checkpoint" : );
    a[0] = 1;
    printf("%d\n", a[0]);
    while(1);

#ifdef CONFIG_SEL4_USE_EMULATION
    asm (
        "mov $60, %%rax;"     // exit is 60
        "xor %%rdi, %%rdi;"    // return code 0
        "syscall;"
        :
        :          
    );
#endif
    return 0;
}


