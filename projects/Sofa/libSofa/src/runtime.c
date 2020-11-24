#include "runtime.h"
#include "helpers.h"
#include <allocman/vka.h>
#include <allocman/bootstrap.h>
#include <sel4runtime.h>
#include <Sofa.h>
#include "syscalls.h"

/* dummy global for libsel4muslcsys */
char _cpio_archive[1];
char _cpio_archive_end[1];


static seL4_CPtr endpoint;
static struct env env;


static TLSContext _mainTLSContext;

seL4_CPtr getProcessEndpoint(void)
{
    return endpoint;
}
struct env* getProcessEnv(void)
{
    return &env;
}




static void process_exit(int code)
{
    sc_exit(getProcessEndpoint(), code);
    // no return
    assert(0);
}

int RuntimeInit2(int argc, char *argv[])
{
    assert(argc == 2);
    endpoint = (seL4_CPtr) atoi(argv[0]);
    test_init_data_t *init_data = (void *) atol(argv[1]);

    memset(&_mainTLSContext, 0, sizeof(TLSContext));
    _mainTLSContext.ep = endpoint;
    _mainTLSContext.buffer = init_data->mainIPCBuffer;
    env.pid = init_data->pid;

    TLSSet(&_mainTLSContext);
    sel4runtime_set_exit(process_exit);

    return 0;
}
int RuntimeInit(int argc, char *argv[])
{
/* parse args */
    assert(argc == 2);
    endpoint = (seL4_CPtr) atoi(argv[0]);

/* read in init data */
    test_init_data_t *init_data = (void *) atol(argv[1]);
    memset(&_mainTLSContext, 0, sizeof(TLSContext));
    _mainTLSContext.ep = endpoint;
    sel4runtime_set_exit(process_exit);

    TLSSet(&_mainTLSContext);

/* configure env */
    env.mainIPCBuffer = init_data->mainIPCBuffer;
    _mainTLSContext.buffer = env.mainIPCBuffer;
    env.vspace_root = init_data->vspace_root;
    env.pid = init_data->pid;
    env.priority = init_data->priority;
    env.cspace_size_bits = init_data->cspace_size_bits;
#ifdef CONFIG_IOMMU
    env.io_space = init_data->io_space;
#endif
#ifdef CONFIG_TK1_SMMU
    env.io_space_caps = init_data->io_space_caps;
#endif
    env.timer_notification.cptr = init_data->timer_ntfn;

    return 0;
}


seL4_CPtr getNewThreadEndpoint(uint8_t** ipcBufferAddr)
{
    return 0;
#if 0
    seL4_CPtr recvSlot;
    int vka_error = vka_cspace_alloc(&getProcessEnv()->vka, &recvSlot);
    assert(vka_error == 0);
    set_cap_receive_path(getProcessEnv(), recvSlot);
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_ThreadNew);

    info = seL4_Call(TLSGet()->ep, info);

    *ipcBufferAddr = (uint8_t*)seL4_GetMR(1);
    return recvSlot;
#endif
}

void sendThreadExit(seL4_CPtr ep)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, SyscallID_ThreadExit);

    seL4_Send(ep, info);
}


void TLSSet( TLSContext* ctx)
{
    seL4_SetUserData((seL4_Word) ctx);
}
TLSContext* TLSGet(void)
{
    return (TLSContext*) seL4_GetUserData();
}