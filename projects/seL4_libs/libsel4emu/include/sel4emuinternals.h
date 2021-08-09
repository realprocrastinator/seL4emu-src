#pragma once

#include <sel4/sel4.h>
#include <sel4emuerror.h>
#include <stdbool.h>
#include <stdint.h>

struct seL4emu_init_state {
  seL4emu_err_t emu_err;
  int sys_errno;
  int sock;
  seL4_BootInfo *bootinfo_ptr;
  seL4_IPCBuffer *ipcbuf_ptr;
};

typedef struct seL4emu_init_state seL4emu_init_state_t;

/**
 * This structure is used during the initial bootstrap of the
 * emulation framework.
 */
extern seL4emu_init_state_t seL4emu_g_initstate;

#define ST_GET_IPCBUFPTR(state) ((&state)->ipcbuf_ptr)
#define ST_GET_BOOTINFOPTR(state) ((&state)->bootinfo_ptr)
#define ST_GET_EMUERR(state) ((&state)->emu_err)
#define ST_GET_SYSERR(state) ((&state)->sys_errno)
#define ST_GET_SOCK(state) ((&state)->sock)

#define ST_SET_IPCBUFPTR(state, ptr) ((&state)->ipcbuf_ptr = ptr)
#define ST_SET_BOOTINFOPTR(state, ptr) ((&state)->bootinfo_ptr = ptr)
#define ST_SET_EMUERR(state, err) ((&state)->emu_err = (err))
#define ST_SET_SYSERR(state, err) ((&state)->sys_errno = (err))
#define ST_SET_SOCK(state, s) ((&state)->sock = s)

// static inline seL4emu_err_t seL4emu_get_errcode(void) {
//   return seL4emu_g_initstate.err;
// }

// static inline void seL4emu_set_errcode(seL4emu_err_t err, bool sys_errno) {
//   if (sys_errno) {
//     seL4emu_g_initstate.sys_errno = err;
//   } else {
//     seL4emu_g_initstate.emu_err = err;
//   }
// }

static inline void seL4emu_initstate(void) {
  seL4emu_g_initstate.sys_errno = 0;
  seL4emu_g_initstate.emu_err = SEL4EMU_INIT_NOERROR;
  seL4emu_g_initstate.sock = -1;
  seL4emu_g_initstate.bootinfo_ptr = NULL;
  seL4emu_g_initstate.ipcbuf_ptr = NULL;
}

/**
 * Any failed in this function will not be return as error code,
 * but will be saved into a global structure, and the next time,
 * when the client wants to access the bootinfo/ipcbuffer, we will
 * fail. Do this because this function is called by the early start
 * and initial routine by seL4runtime, and there is no where we can
 * report the error and exit.
 */
void seL4emu_internal_setup(seL4_BootInfo **bootinfo, seL4_IPCBuffer **ipc_buffer);
