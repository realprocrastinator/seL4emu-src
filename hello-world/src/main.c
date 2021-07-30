#include <autoconf.h>
#include <stdio.h>
#include <unistd.h>

#ifdef CONFIG_SEL4_USE_EMULATION
#define _GNU_SOURCE
#include <mini_signal.h>

static void sigsegvhdlr(int sig, siginfo_t* info, void* ctx) {
    printf("SEGV!\n");
    _exit(0); // halt in a infinite loop
}

static void install_sigsegv_handler() {
    struct sigaction sa = {0};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigsegvhdlr;
    mini_sigaction(SIGSEGV, &sa, NULL);
}
#endif

int main(int argc, char *argv[]) {

#if CONFIG_SEL4_USE_EMULATION
    install_sigsegv_handler();
#endif

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


