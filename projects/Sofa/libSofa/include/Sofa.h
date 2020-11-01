#pragma once
#include <sys/types.h>
#include <unistd.h>
#include <sel4/sel4.h>
#include <proc_ctx.h>
#include <time.h>
/* API that should go at some point in the runtime in order to be called before/after main()*/

int ProcessInit(seL4_CPtr endpoint);


seL4_CPtr RequestCap(int index);
ProcessContext* getProcessContext(void);

seL4_CPtr registerIPCService(const char* name, seL4_CapRights_t rights);
seL4_CPtr getIPCService(const char* name);

void DoDebug(int code);

void tempSetTimeServerEP( seL4_CPtr ep);
/* PUBLIC API*/

pid_t waitpid(pid_t pid, int *wstatus, int options);
pid_t wait(int *wstatus);

pid_t getpid(void);
pid_t getppid(void);

int nanosleep(const struct timespec *req, struct timespec *rem);


TLSContext* getTLSContext(void);

