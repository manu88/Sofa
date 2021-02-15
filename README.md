# Sofa

Sofa is built atop SeL4 microkernel and provides a set of userland servers and an API to manipulate them.

## Features:
### System:
* IPC Model between root server ('*kernel_task*') and applications
* Thread in processes
* UDP stack with virtio device
* virtio-pci-blk driver (IRQ Not working, polling only for now)

### Process API
* printf
* getpid, getppid,
* spawn,
* sleep,
* wait/waitpid,
* kill,
* readdir
* Name server: register service, get service by name,
* threads
* VFS: mount, open, close, read
* Net: socket, bind, recvfrom, sendto
* module loader

### POSIX status
Because we use `musl`, the current goal is to be as much as possible POSIX compliant. Except for:
* `fork`, that might never be implemented. [here](https://www.microsoft.com/en-us/research/uploads/prod/2019/04/fork-hotos19.pdf)'s why.

## Applications
* init
* shell
* udp echo server

## Services
* Process: Enum, Spawn, Kill
* VFS (read-only for now)
* net (udp only)
* DeviceKit
* init: pretty much empty for now, serves as a use case to implement servers

## Build
See [dependencies.md](dependencies.md) for a list of required software.


You should have all the tools and external deps required to build seL4 (some info [here](https://docs.sel4.systems/projects/buildsystem/host-dependencies.html)).

Then call the update.sh script to fetch all the required components (seL4 kernel, libs, etc.)

**Important**

The `LibSel4MuslcSysMorecoreBytes` config var *must be* set to `0` in the project configuration. This can be done using `ccmake ../projects/Sofa/` in the build folder.

```
sh update.sh
mkdir build
cd build
../init-build.sh  -DPLATFORM=x86_64 -DSIMULATION=TRUE -DRELEASE=FALSE -DLWIP_DEBUG=1 -DLWIP_PATH=../projects/libliwip/
ninja
./simulate --extra-qemu-args "-netdev user,id=net1,hostfwd=udp::3000-:3000 -device virtio-net-pci,netdev=net1 -device virtio-blk-pci,drive=drive0 -drive file=../sysroot.ext2.qcow2,if=none,id=drive0"
```

### Create a qcow2 image
from [here](https://serverfault.com/questions/246835/convert-directory-to-qemu-kvm-virtual-disk-image):

*Note*: to mount and edit the volume on the host: see https://gist.github.com/shamil/62935d9b456a6f9877b5.


## Test the UDP stack:
start the `udpecho` program and test it by sending messages from the host:
```
nc  -u 127.0.0.1 3000
```

## Create an ext2 distribution image:
after build, in the `dist` folder:
```
sudo sh build.sh
# on sucess:
qemu-system-x86_64 -cpu Nehalem,-vme,+pdpe1gb,-xsave,-xsaveopt,-xsavec,-fsgsbase,-invpcid,enforce -device virtio-blk-pci,drive=drive0 -drive file=sofa.iso,if=none,id=drive0 -serial stdio
```

## Resources
* [camkes-VM](https://github.com/seL4/camkes-vm/blob/master/components/Init/src/main.c),
* [sel4test](https://github.com/seL4/sel4test),
* [libsel4osapi](https://github.com/rticommunity/libsel4osapi)
* [Virtio devices](https://wiki.osdev.org/Virtio)
