# Implementation notes

## kernel_task : the Root Server
This is the first user space process started by the kernel, It's tasks are:

* To boostrap the rest of the system ( mostly delegated by the 'init' process).
* To handle all syscalls.
* And to handle interupts from the kernel.


## init : the first process


## Issuing a syscall