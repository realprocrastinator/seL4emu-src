#define _GNU_SOURCE
#include <autoconf.h>
#include <stdio.h>
#include <unistd.h>
#include <mini_signal.h>

static void sigsegvhdlr(int sig, siginfo_t* info, void* ctx) {
    printf("SEGV!\n");
    _exit(0); // halt in a infinite loop
}

int main(int argc, char *argv[]) {
    
    struct sigaction sa = {0};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigsegvhdlr;
    mini_sigaction(SIGSEGV, &sa, NULL);

    printf("Hello, World!\n");

    // _exit(0); halt in a infinite loop

    // void *a = (void *)0xffffff8000000000;
    // *(char *)a = 1;


// #ifdef CONFIG_SEL4_USE_EMULATION
//     asm (
//         "mov $60, %%rax;"     // exit is 60
//         "xor %%rdi, %%rdi;"    // return code 0
//         "syscall;"
//         :
//         :          
//     );
// #endif
    return 0;
}


