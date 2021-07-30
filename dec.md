## Implementation 

## (DRAFT version only for me to remember things!)

### Notice

The server is the kernel emulator

The clients are seL4 applications, such as the roottask and the other seL4 threads launched by the roottask.

### Build Systems

Reused the current seL4 build systems for both building the server and the client. The reason is try to use as much configuration as much as possible. The current build systems will automatically generate the header files and the invocation interfaces for the seL4 system. One advantage is that we don't worry about building our own build systems which can take a lot of time. The other thing is if any configuration or interfaces changed in the upstream, we can easily merge the updates to the current code base.

The emulation build is implemented by adding an configuration option at the top level kernel `config.cmake`. The flag named `seL4UseEmu` will be cached in the `CMakeCache.txt` in the build directory for switching between building the emulation system or the original seL4 system.

The current demo project is forks the seL4 tutorials, so we can use the seL4 tutorial to initialize the environment and the build the demo applications.

Future work will be modifying the build systems to support more generic building of a project to use the emulation system.      

### Server

The server will handle all the syscalls issued by the sle4 client apps by emulation via Linux syscalls.

It should be able to handle multi-clients' requests. Considering using `epoll`. 

`UDS` will be bound to a path. Once bound can't be rebound again, it must be removed explicitly.

For sel4 threads, we need to distinguish the *threads with same vspace* and the *threads with dffferent vspaces*.

The server needs to pre-mapped a virtual memory     

### Client Emu Lib

The client is statically linked with `musllibc`, to avoid conflicting, we need to use as less `stdlib` functions as possible. 

Client sends one `char`, then on the server side, it receives one `char` then send it back to client.   

The current emulation targets at the arch-dependent syscall interfaces. Reason is because we change as less code as possible. Those functions prefixed with the arch identifiers are the core functions implement the syscall ABIs. However, since all the interfaces have same parameters, we can easily make it generic for all the architecture supported in the seL4 system.

NOTICE: The current implementation has one drawbacks is that the users who wants to use the emulation frameworks have to use the libseL4, and the emulation framework is implemented in the libseL4 instead of being a standalone library. In the future, a better way can be let seL4 kernel build systems to generate a standalone libseL4 emulation library for usage depending on the configuration provided.    

### Custom Syscall Wrapper Functions

Since the seL4 system are liked with musllibc, to implement the emulation, we need to use the syscalls, so here, we need to implement minimal wrappers of the raw syscalls.

Currently, supported functions are signals handling functions, and io functions.

### Implementation Details

#### seL4Runtime

During the seL4 roottask starts up phase. There are several important things happening. 

#### Set up IPCBuffer

First of all is to obtain the boot information which is passed by the seL4 kernel. The most important information the roottask needs is the  some initial caps to the kernel objects such as the untyped memory and the pointer to the initial IPCbuffer. Notice that in the real seL4 system, the kernel maps the initial IPCBuffer for the roottask, to emulate this we can staticly assign an initial IPCbuffer during the runtime for ease. 

A better solution can be let the kernel emulator parse the ELF file of the roottask and obtain the first avalaible address where can actually map the IPCBuffer then pass it in the bootinfo when to the roottask is trying to launch it, and the roottask map it to its address space sliently when it first touched the IPCBuffer.       

#### Set up TLS region

In the real seL4 system, the TLS region is used for each seL4threads to store data such as the pointer to the IPCBuffer etc. And the seL4 also has explicit syscall interfaces to setup the TLS region. While this should be handled by the kernel itself . However for emulation we handle it on the client side as the client and the server are separated processes, the server can't change the clildren process's fs register unless using ptrace.  The other thing is that, to change fs register in the user space, we can utilise the fsgsbase instruction familly in the x86. Otherwise we can use the Linux syscall to do that.

However whatever which way we do, we need to inform the kenrel emulator to bookkeep the biframe and ipbbuffer in the roottask's vspace.
