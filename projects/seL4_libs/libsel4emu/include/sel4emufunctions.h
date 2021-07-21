#pragma once

#include <sel4/types.h>
// #include <sel4/syscalls.h>

// extern __thread seL4_IPCBuffer *__sel4_ipc_buffer;

#ifdef CONFIG_KERNEL_INVOCATION_REPORT_ERROR_IPC
// extern __thread char __sel4_print_error;

char *seL4emu_GetDebugError(void);

void seL4emu_SetPrintError(char print_error);

char seL4emu_CanPrintError(void);
#endif

void seL4emu_SetIPCBuffer(seL4_IPCBuffer *ipc_buffer);

seL4_IPCBuffer *seL4emu_GetIPCBuffer(void);

seL4_Word seL4emu_GetMR(int i);

void seL4emu_SetMR(int i, seL4_Word mr);

seL4_Word seL4emu_GetUserData(void);

void seL4emu_SetUserData(seL4_Word data);

seL4_Word seL4emu_GetBadge(int i);

seL4_CPtr seL4emu_GetCap(int i);

void seL4emu_SetCap(int i, seL4_CPtr cptr);

void seL4emu_GetCapReceivePath(seL4_CPtr *receiveCNode, seL4_CPtr *receiveIndex,
                                                seL4_Word *receiveDepth);

void seL4emu_SetCapReceivePath(seL4_CPtr receiveCNode, seL4_CPtr receiveIndex, seL4_Word receiveDepth);