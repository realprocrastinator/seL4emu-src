<!--
     Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)

     SPDX-License-Identifier: CC-BY-SA-4.0
-->

# libsel4rpc

Libsel4rpc provides a basic library for managing allocating resources over
process boundaries. It provides a protobuf-based protocol definition, and a few
simple wrappers to ease receiving/replying to RPC messages on the server side,
and sending RPC requests on the client side.

Resource allocation across process boundaries is reasonably common in seL4,
where we often have a "parent" process (e.g. sel4test's driver), and a "child"
process (e.g. sel4test's tests). Previously, if the child process wanted a
resource (e.g. an IRQ, or a specific frame of memory), the parent had to know
in advance and supply the resource(s) to the parent process to the child at
process creation time.

Libsel4rpc provides a simple way to remove the need for allocation in advance,
and allows the parent to provide capabilities to the child process on demand.

For example, a small sample is below...

Server:
```c
// initialise the server
sel4rpc_server_env_t rpc_server;
sel4rpc_server_init(&rpc_server, vka, my_handler_function, my_data,
        &env->reply, simple);

// the main RPC loop
while (1) {
    seL4_MessageInfo_t info = api_recv(process_ep, &badge, env->reply.cptr);
    if (seL4_GetMR(0) == blah) {
        sel4rpc_server_recv(&rpc_server);
    }
}
```

Client:
```c
// initialise the client
sel4rpc_client_t rpc_client;
sel4rpc_client_init(&rpc_client, server_endpoint);

// make an RPC request
// create the request object: allocate an untyped object at address 0x10440000
// this will be encoded into the process's IPC buffer by sel4rpc_call
// and sent to the server.
RpcMessage msg = {
    .which_msg = RpcMessage_memory_tag,
    .msg.memory = {
        .address = 0x10440000,
        .size_bits = 12,
        .type = seL4_UntypedObject,
    },
};

// set up somewhere to put our received capability
cspacepath_t path;
int ret = vka_alloc_path(vka, &path);
if (ret)
    ZF_LOGF("Failed to alloc path: %d", ret);

// call, putting the response cap (if there is one) into the cap
// represented by 'path'.
ret = sel4rpc_call(&rpc_client, &msg, path.root, path.capPtr, path.capDepth);
if (ret)
    ZF_LOGF("RPC call failed: %d", ret);

int errorCode = msg.msg.errorCode;
if (errorCode)
    ZF_LOGF("RPC alloc request failed: %d", errorCode);

// at this point, the cap was successfully allocated and is ready to use.
```
