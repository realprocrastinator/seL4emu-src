## Implementation

### Server

The server will handle all the syscalls issued by the sle4 client apps by emulation via Linux syscalls.

It should be able to handle multi-clients' requests. Considering using `epoll`. 

`UDS` will be bound to a path. Once bound can't be rebound again, it must be removed explicitly.

For sel4 threads, we need to distinguish the *threads with same vspace* and the *threads with dffferent vspaces*.

The server needs to pre-mapped a virtual memory     

### Client Emu Lib

The client is statically linked with `musllibc`, to avoid conflicting, we need to use as less `stdlib` functions as possible. 

Client sends one `char`, then on the server side, it receives one `char` then send it back to client.   

### Custom Syscall Wrapper Functions

Since the seL4 system are liked with musllibc, to implement the emulation, we need to use the syscalls, so here, we need to implement minimal wrappers of the raw syscalls.

