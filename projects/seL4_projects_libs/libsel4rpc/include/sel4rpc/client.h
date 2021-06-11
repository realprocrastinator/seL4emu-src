/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <sel4/sel4.h>

struct _RpcMessage;
typedef struct _RpcMessage RpcMessage;

typedef struct sel4rpc_client_env {
    seL4_CPtr server_ep;
    seL4_Word magic;
} sel4rpc_client_t;

int sel4rpc_client_init(sel4rpc_client_t *client, seL4_CPtr server_ep, seL4_Word magic);
int sel4rpc_call(sel4rpc_client_t *client, RpcMessage *msg, seL4_CPtr root,
                 seL4_CPtr capPtr, seL4_Word capDepth);
