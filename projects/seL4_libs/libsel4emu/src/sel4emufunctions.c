/*
 * This is the emulation stubs of libsel4/sel4/functions.h
 */

#include <sel4/types.h>
#include <sel4emusyscalls.h>
#include <sel4emufunctions.h>

extern seL4_IPCBuffer *__sel4_ipc_buffer;

#ifdef CONFIG_KERNEL_INVOCATION_REPORT_ERROR_IPC
extern char __sel4_print_error;

char *seL4emu_GetDebugError(void)
{
    return (char *)(__sel4_ipc_buffer->msg + DEBUG_MESSAGE_START);
}

seL4emu_SetPrintError(char print_error)
{
    __sel4_print_error = print_error;
    return;
}

char seL4emu_CanPrintError(void)
{
    return __sel4_print_error;
}
#endif

void seL4emu_SetIPCBuffer(seL4_IPCBuffer *ipc_buffer)
{
    __sel4_ipc_buffer = ipc_buffer;
    return;
}

seL4_IPCBuffer *seL4emu_GetIPCBuffer(void)
{
    return __sel4_ipc_buffer;
}

seL4_Word seL4emu_GetMR(int i)
{
    return seL4emu_GetIPCBuffer()->msg[i];
}

void seL4emu_SetMR(int i, seL4_Word mr)
{
    seL4emu_GetIPCBuffer()->msg[i] = mr;
}

seL4_Word seL4_GetUserData(void)
{
    return seL4emu_GetIPCBuffer()->userData;
}

void seL4emu_SetUserData(seL4_Word data)
{
    seL4emu_GetIPCBuffer()->userData = data;
}

seL4_Word seL4emu_GetBadge(int i)
{
    return seL4emu_GetIPCBuffer()->caps_or_badges[i];
}

seL4_CPtr seL4emu_GetCap(int i)
{
    return (seL4_CPtr)seL4emu_GetIPCBuffer()->caps_or_badges[i];
}

LIBSEL4_INLINE_FUNC void seL4_SetCap(int i, seL4_CPtr cptr)
{
    seL4emu_GetIPCBuffer()->caps_or_badges[i] = (seL4_Word)cptr;
}

void seL4emu_GetCapReceivePath(seL4_CPtr *receiveCNode, seL4_CPtr *receiveIndex,
                                                seL4_Word *receiveDepth)
{
    seL4_IPCBuffer *ipcbuffer = seL4emu_GetIPCBuffer();
    if (receiveCNode != (void *)0) {
        *receiveCNode = ipcbuffer->receiveCNode;
    }

    if (receiveIndex != (void *)0) {
        *receiveIndex = ipcbuffer->receiveIndex;
    }

    if (receiveDepth != (void *)0) {
        *receiveDepth = ipcbuffer->receiveDepth;
    }
}

void seL4emu_SetCapReceivePath(seL4_CPtr receiveCNode, seL4_CPtr receiveIndex, seL4_Word receiveDepth)
{
    seL4_IPCBuffer *ipcbuffer = seL4emu_GetIPCBuffer();
    ipcbuffer->receiveCNode = receiveCNode;
    ipcbuffer->receiveIndex = receiveIndex;
    ipcbuffer->receiveDepth = receiveDepth;
}

