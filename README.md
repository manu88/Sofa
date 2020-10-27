# Sofa

Sofa is build atop SeL4 microkernel and provides a set of userland servers and an API to manipulate them.

## Status
for now I'm trying to understand more the [sel4test](https://github.com/seL4/sel4test) repo and the [libsel4osapi](https://github.com/rticommunity/libsel4osapi) repo in order to create a process model with memory management.

## Features:
### System:
* IPC Model between root server ('kernel_task') and applications
* process spawn from CPIO archive **[In progress, lack the multi process part in kernel_task]**
* Thread in processes **[In progress]**

### Process API
* printf
* getpid, getppid,
* posix_spawn
* sleep

