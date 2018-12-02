# Sofa

Sofa is a test bed project for implementing an Operating System on top of the SeL4 microkernel.

Usefull links :

* [Advanced Operating Systems COMP9242 2018/S2 Course](http://www.cse.unsw.edu.au/~cs9242/)
* [SeL4 API Reference](https://docs.sel4.systems/ApiDoc.html#signal)
* [Sofa implementation notes](https://github.com/manu88/Sofa/blob/master/ImplementationNotes.md)

## (Loosely & inefficiently) implemented SysCalls
* `getpid`
* `getppid`
* `nanosleep`
* `kill`
*  kind of `execve` that starts a new process, but does not work as expected since it does not replace current process. This is a combinaison of fork+execve
* `wait4`
* `open` and  `close` in Progress!
* `read` and `seek` (almost) working on CPIO files
* `write`
* `clock_gettime`
* `getcwd`
* `chdir`
* `fcntl`
* `getdents64`
* `mkdir`
* `stat`
* `get(set)prioriy`

## Features
* EGA shell
* Unified File System with CPIO mount point, `dev` and `proc` folders

## How to build

Create a build folder in the root dir :

```bash
mkdir build
```
initialize the build system

```bash
../init-build.sh  -DPLATFORM=x86_64 -DSIMULATION=TRUE
```

Then start the build process

```bash
ninja
```
