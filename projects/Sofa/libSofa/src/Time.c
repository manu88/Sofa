#include "sys_calls.h"
#include "Sofa.h"


static seL4_CPtr _timeEp = 0;
void tempSetTimeServerEP( seL4_CPtr ep)
{
    _timeEp = ep;
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    TLSContext* ctx = getTLSContext();

    seL4_MessageInfo_t msg = seL4_MessageInfo_new(seL4_Fault_NullFault, 0,0,3);
    seL4_SetMR(0, TimeServerSysCall_Sleep);
    seL4_SetMR(1, req->tv_sec);
    seL4_SetMR(2, req->tv_nsec);

    seL4_Call(_timeEp, msg);
    printf("nanosleep returned\n");
    return -EPERM;
}