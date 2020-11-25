# Sofa

Sofa is build atop SeL4 microkernel and provides a set of userland servers and an API to manipulate them.

## Build
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


## Test the UDP stack:
You can send udp messages using:
```
nc  -u 127.0.0.1 3000
# or
echo "test2" > /dev/udp/127.0.0.1/3000
```

## Features:
### System:
* IPC Model between root server ('*kernel_task*') and applications
* process spawn from CPIO archive
* Thread in processes
* UDP stack with virtio device

### Process API
* printf
* getpid, getppid,
* spawn
* sleep
* wait/waitpid
* kill
* Name server: register service, get service by name

## Applications
* init
* shell

## Resources
* [camkes-VM](https://github.com/seL4/camkes-vm/blob/master/components/Init/src/main.c),
* [sel4test](https://github.com/seL4/sel4test),
* [libsel4osapi](https://github.com/rticommunity/libsel4osapi)
* [Virtio devices](https://wiki.osdev.org/Virtio)
