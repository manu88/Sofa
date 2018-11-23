# Implementation notes

## kernel_task : the Root Server
This is the first user space process started by the kernel, It's tasks are:

* To boostrap the rest of the system ( mostly delegated by the 'init' process).
* To handle all syscalls.
* And to handle interupts from the kernel.

### DriverKit
DriverKit is an object oriented framework for hardware communication. Everything from SeL4 should be abstracted and opaque from an `IOBaseDevice` perspective point.

### FileServer

### DevServer

### ProcessTable


## init : the first process


## Issuing a syscall