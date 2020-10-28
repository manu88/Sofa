#pragma once
#include <sys/types.h>
#include <unistd.h>

/* API that should go at some point in the runtime in order to be called before/after main()*/

int ProcessInit(void* endpoint);




/* PUBLIC API*/

pid_t waitpid(pid_t pid, int *wstatus, int options);
pid_t wait(int *wstatus);

pid_t getpid(void);
pid_t getppid(void);
