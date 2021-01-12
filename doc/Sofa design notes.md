# Sofa design notes

### Sel4 and root task

Sofa is designed with *isolation* in mind. Each process runs in a hierarchy of processes, with a common ancestor, the kernel_task (**Rename to ProcessServer?**)

* kernel_task
  * process pid 1
  * process pid 2
  * ...

the kernel_task is responsible for:

* Start the system after kernel initialization,
* Allocating memory for processes,
* Creating new threads,
* Handling service registration and connections from clients, and
* Other syscalls such as `sleep` or `gettime` that do not belong in any services for now, but might be in the future.



### Services/Servers [choose ONE name :)]

Sofa provides the ability to run Servers that handles and processes other clients requests. The concept is inspired from Mach Named servers.

A server can:

* Register itself for a specific *Name*,
* Listen to incoming client requests using the endpoint provided on registration



A Client can:

* Connect to a specific server, by *name*,
* Send Messages to the server

Sofa does not enforce any design and protocol choice for the Service, except for:

* The Connection, that always be made from Client to Service. In other words, a service can't send messages to clients without having received a message from client first.
* The kernel_task server will send specific messages to the service for the following events:
  * A Connected client had `exit`,
  * The Service will be stopped,
  * Process clone request, see **Fork & Spawn** 



### Fork & Spawn

Fork semantics requires an entire process to be cloned, which may be hard to design in the context of a micro-kernel where a process state is distributed along many services. 

For now, Sofa does not provide a `fork`-like syscall, and relies on the `spawn` syscall for creating new processes.

However, spawn (in the Posix sense) still requires some part of the calling process to be cloned, notably the opened files. Thats why Sofa will, when spawn is invoked:

* Iterate over all Services the calling process is a client,
* For each service that has the `ServiceFlag_Clone`flag set, send a message to the service to clone the state of the parent into the new child.
* Such clone operation is entirely left to the service and Sofa does not enforce any policy regarding the cloning logic.

#### VFSService Client clone example 

The clone method can  be found in the VFSService  around [here](https://github.com/manu88/Sofa/blob/master/projects/Sofa/kernel_task/src/VFSService.c#L235).

A VFSService client has the following properties:

* A list of open files
* The current working directory

On clone request, the Service will simply copy the working directory and the open file list into the new process.



On the contrary, the Process Service [ProcService](https://github.com/manu88/Sofa/blob/master/projects/Sofa/kernel_task/src/ProcService.c) will not clone any client, simply because it has no state.





