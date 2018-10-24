# Sofa

Sofa is a test bed project for implementing an Operating System on top of the SeL4 microkernel.

Usefull links :

* [Advanced Operating Systems COMP9242 2018/S2 Course](http://www.cse.unsw.edu.au/~cs9242/)
* [SeL4 API Reference](https://docs.sel4.systems/ApiDoc.html#signal)

## (Loosely & inefficiently) implemented SysCalls
* `getpid`
* `getppid`
* `nanosleep`
* `kill`
*  kind of `execve` that starts a new process, but does not work as expected since it does not replace current process. This is a combinaison of fork+execve
* `wait4`
