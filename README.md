# Sofa

Sofa is build atop SeL4 microkernel and provides a set of userland servers and an API to manipulate them.

## Build
You should have all the tools and external deps required to build seL4 (some info [here](https://docs.sel4.systems/projects/buildsystem/host-dependencies.html)).

Then call the update.sh script to fetch all the required components (seL4 kernel, libs, etc.)

```
sh update.sh
mkdir build
cd build
../init-build.sh  -DPLATFORM=x86_64 -DSIMULATION=TRUE -DRELEASE=FALSE
ninja
./simulate
```

## Status
for now I'm trying to understand more the [sel4test](https://github.com/seL4/sel4test) repo and the [libsel4osapi](https://github.com/rticommunity/libsel4osapi) repo in order to create a process model with memory management.

## Features:
### System:
* IPC Model between root server ('kernel_task') and applications
* process spawn from CPIO archive **[In progress, lack the multi process part in kernel_task]**
* Thread in processes **[In progress]**
* TimeServer to handle time related stuff. This will hopefully become an generic IO service that spawn processes that act like drivers.

### Process API
* printf
* getpid, getppid,
* posix_spawn
* sleep
* wait/waitpid

