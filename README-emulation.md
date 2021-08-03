# Emulation Building

This branch is modified based on the seL4 tutorial repo to support the emulation framework. The purpose of the emulation framework is to emulate the seL4 system in the Linux userspace. The seL4 system has a seL4 kernel which provides the minimal kernel service, such as cspace, vspace, tcb scheduling management, etc. On the other hand, it also has a `roottask` which is booted by the seL4 kernel and takes over most of the OS service management as well as other seL4 threads created by the `roottask`. In the emulation framework, we also want to follow this architecture of the system, but we map each seL4 threads to a Linux process, and we implement the seL4 kernel as a separate Linux process running like a server.     

## Notice

1. The project currently targets x86_64 architecture, any other architecture can result in building failure.
2. The project has been tested on Ubuntu 21.04, Linux kernel version 5.12.17, However it should also work on the earlier version of Linux (TODO: minimal version).  

## Starting the emulation building

The current implementation leverage the existing seL4 build system to make building and configuration easier. Therefore, there are limited existed applications where we can use. But with the build system, it is also easy to create our own demo applications.

Before building, please make sure all the host dependencies have been installed. Please follow the instructions for setting up your host environment on the [seL4 docsite](https://docs.sel4.systems/HostDependencies).

1. build the tutorial applications

```sh
git clone https://github.com/realprocrastinator/seL4-emulation.git
cd seL4-emulation
# To list all the tutorials by running
./init --help 
# To build hello application
./init --plat pc99 --tut hello-world
# This will generate two folders
# The hello-world/ contains the source code of the hello_world tutorial
# The hello-world_build/ will contain the building results
```

2. build the original seL4 system

```sh
cd hello-world_build
ninja
# test
./simulate
```

3. build with emulation configuration

With the modification of the build system, CMake will cache a configuration option called `seL4UseEmu`,  in the `CMakeCache.txt` in the build directory. By default it's disabled. We can tune the option from `Off`  to `On` to build with emulation configuration.

```sh
# Make sure in the build directory, in our case it's hello-world_build
# Change `seL4UseEmu` to On in CMakeCache.txt
ninja
# This will generate a folder called emu-images which contains the `roottask` image and the kernel emulator image
# To test the building, we can run the kernel emulator directly.
./kernel-x86_64-pc99-emu
```

At this point, the emulation framework has been built successfully. However, the application will fail at a certain point due to the incomplete kernel emulator. The current version is quite hacky, especially on the server-side. However, I'm still working on it to improve this.

# sel4-tutorials

Various tutorials for using seL4, its libraries, and CAmkES.

## Prerequisites

Follow the instructions for setting up your host environment on the [seL4 docsite](https://docs.sel4.systems/HostDependencies).

## Starting a tutorial

A tutorial is started through the use of the `init` script that is provided in the root
directory. Using this script you can specify a tutorial and target machine and it will
create a copy of the tutorial for you to work on.

Example:

```sh
mkdir build_hello_1
cd build_hello_1
../init --plat pc99 --tut hello-1
```

The `init` script will initialize a build directory in the current directory and at the end
it will print out a list of files that need to be modified to complete the tutorial. Building
is performed simply be invoking `ninja`, and once the tutorial compiles it can be tested
in Qemu by using the provided simulation script through `./simulate`

## Tutorials and targets

The `-h` switch to the `init` script provides a list of different tutorials and targets that
can be provided to `--plat` and `--tut` respectively.

Most tutorials support any target platform, with the eception of hello-camkes-timer, which only
supports the zynq7000 platform.

## Solutions

To view the solutions for a tutorial instead of performing the tutorial pass the `--solution` flag
to the `init` script

Example:

```sh
mkdir build_hello_1_sol
cd build_hello_1_sol
../init --plat pc99 --tut hello-1 --solution
```

After which it will tell you where the solution files are that you can look at. You can then
do `ninja && ./simulate` to build and run the solution.

# Documentation

## Developer wiki

A walkthrough of each tutorial is available on the [`docs site`](https://docs.sel4.systems/Tutorials)

## Tutorial Slides

The slides used for the tutorial are available in [`docs`](docs).

## seL4 Manual

The seL4 manual lives in the kernel source in the [`manual`](https://github.com/seL4/seL4/tree/master/manual) directory.
To generate a PDF go into that directory and type `make`.
You will need to have LaTeX installed to build it.

A pre-generated PDF version can be found [`here`](http://sel4.systems/Info/Docs/seL4-manual-latest.pdf).

## CAmkES Documentation

CAmkES documentation lives in the camkes-tool repository in [docs/index.md](https://github.com/seL4/camkes-tool/blob/master/docs/index.md).

## Emulation Implementation

TODO