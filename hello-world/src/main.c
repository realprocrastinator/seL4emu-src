/* This application is only used for testing cli-emualtion framework */
#include <autoconf.h>
#include <stdio.h>

#ifdef CONFIG_SEL4_USE_EMULATION
#define _GNU_SOURCE
#include <mini_assert.h>
#include <mini_signal.h>
#include <mini_syscalls.h>
#include <string.h>
#include <sys/mini_mman.h>
#include <unistd.h>

#define TEST_ADDR 0xffffff80000UL

/* custom helpers */
#define MAKE_CHECKPOINT asm volatile("checkpoint: .global checkpoint" :)
#define LOAD_CHECKPOINT asm volatile("movabs $checkpoint, %0" : "=r"(*rip))

typedef void (*handler_func)(int sig, siginfo_t *info, void *ctx);

/* stack for handling signals */
static char signal_stack[SIGSTKSZ];

static void sigsegvhdlr(int sig, siginfo_t *info, void *ctx) {
  ucontext_t *uc = (ucontext_t *)ctx;
  greg_t *rip = &uc->uc_mcontext.gregs[REG_RIP];

  if (sig == SIGSEGV) {
    printf("SEGV @%lx!\n", (unsigned long)info->si_addr);
    // check align with one page;
    mini_assert((((unsigned long)info->si_addr) & (0x1000 - 1)) == 0);
    char *addr =
        (char *)mini_mmap(info->si_addr, 4096, PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    if (addr == MAP_FAILED) {
      mini_perror("mini_mmap");
      _exit(0); // halt in a infinite loop
    }
    printf("New mapping addr @%p.\n", addr);
    mini_assert((((unsigned long)info->si_addr) & ~(0x1000 - 1)) ==
                (unsigned long)addr);

  } else {
    fprintf(stderr, "Expect SIGSEGV but caught signal: %d\n", sig);
    _exit(1);
  }
}

static int setup_sig_stack(void *sptr, size_t size) {
  int ret = 0;

  // set up the signal stack
  stack_t ss;
  ss.ss_sp = sptr;
  ss.ss_size = size;
  ss.ss_flags = 0;

  if (mini_sigaltstack(&ss, NULL) < 0) {
    perror("mini sigaltstack");
    ret = -1;
  }

  return ret;
}

static void touch_and_verify_page(void *addr) {
  char *a = (char *)addr;

  for (int i = 0; i < 4096; ++i) {
    a[i] = 'a';
  }

  for (int i = 0; i < 4096; ++i) {
    mini_assert(a[i] == 'a');
  }

  printf("mmap succeed!\n");
}

static void test_fault_and_map(void) {
  /* set up a static stack for handling signals */
  if (setup_sig_stack(signal_stack, sizeof(signal_stack)) < 0) {
    fprintf(stderr, "Failed to setup stack\n");
    _exit(1);
  }

  /* setup signal handler for SIGSEGV */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sigset_t mask;
  mini_sigemptyset(&mask);

  sa.sa_flags = SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
  sa.sa_sigaction = sigsegvhdlr;
  sa.sa_mask = mask;

  if (mini_sigaction(SIGSEGV, &sa, NULL) < 0) {
    perror("mini sigaction");
    _exit(1);
  }

  /* generate vm fault and mapp one page implicitly, addr must be 4k aligned! */
  touch_and_verify_page((void *)TEST_ADDR);
}
#endif

int main(int argc, char *argv[]) {

#ifdef CONFIG_SEL4_USE_EMULATION
  test_fault_and_map();
#endif

  printf("Hello, World!\n");

  /* The current runtime lib doesn't implement exit(), we will get segv here! */
  return 0;
}
