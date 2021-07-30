#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>

/* This program is only used for testing whether the build system works */

int main () {
  printf("Hello World from the kernel emulator.\n");
  
  char *a = malloc(4096);
  if (!a) {
    perror("malloc");
    exit(1);
  }  
  
  for (int i = 0; i < 4096; ++i) {
    a[i] = 'a';
  }
  
  return 0;
}