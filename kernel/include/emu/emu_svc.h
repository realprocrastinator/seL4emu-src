#pragma once

#include <emu/emu_ipcmsg.h>

int seL4emu_handle_client_ipc(int sock, seL4emu_ipc_message_t *msg);