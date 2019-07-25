# Sofa OS

Sofa is build atop SeL4 microkernel and provides a set of userland servers and an API to manipulate them.

## Projet philosophy
Sofa is a capability based operating system, built atop a microkernel. Everything is server-based and rely on IPC communications and Virtual Memory.  

The process 'kernel_task' is the first to be spawned ( it is the rootserver in SeL4 terminology). It will then start the 'init' process which will handle the rest of the boot process.

### Client-Server infrastructure 


## Projet layout
### kernel
the SeL4 microkernel.

### kernel_task
the rootserver in the SeL4 terminology, ie. the first userland program to run after the kernel is initialized. Kernel_task provides a basic set of syscalls for other programs to boostrap:

* spawn : starts a new process,
* kill : terminates a program,
* wait : wait for any child to terminate,
* sleep : well... sleeps,
* get wall time
* other debug syscalls, such as printing the scheduler state.



In addition to these basic syscalls, kernel_task acts like a _Name Server_ for _IPC communication_ between programs. All programs can:

* register themselves as an IPC server with an unique identifier.
* connect to an IPC server, given its name.

All spawned programs are able to make these syscalls with the libSysCall library (see below).

Note that even if kernel_task is the fisrt program to start, its pid is 0 and it is hidden to other programs.

### libSysCall
A library programs must link against in order to talk to kernel_task. 

### seL4rt
A modified version of the sel4runtime library from SeL4. Eventually libSysCall and seL4rt will be merged as to provide an unified C runtime library.

### init
The first program (after kernel_task of course) to run, its goal is to bootstrap the rest of the system (DriverKitD and VFSD for example).

### DriverKitD
DriverKitD will create a device Tree from the platform configuration, and try to associate each device with a list a Drivers. These drivers can be userland programs or modules (not dynamic!) inside DriverKitD. 

### VFSD (todo)
A Virtual File System server.


## Driver model
DriverKit is an 'object oriented' drivers framework that allows the creation of userland drivers. These drivers are automatically started when a matching device is detected by the system. 'Platform expert' (eg. PCI) drivers can also be embedded inside DriverKitD.


## build
Fist call the update.sh script to fetch all the required components (seL4 kernel, libs, etc.)

```
sh update.sh
mkdir build
cd build
../init.sh -DSIMULATION=ON
ninja
./simulate
```