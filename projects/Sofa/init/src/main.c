/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <allocman/vka.h>
#include <allocman/bootstrap.h>
#include <Sofa.h>
#include <Thread.h>
#include <runtime.h>
#include <proc.h>
#include <sys/wait.h>

#include <sel4utils/helpers.h>
#include <sel4runtime.h>
#include <sel4utils/thread.h>

static seL4_CPtr endpoint = 0;
static test_init_data_t *init_data = NULL;

typedef struct
{
    seL4_CPtr tcb;
    seL4_CPtr ep;
    seL4_CPtr ipcBuf;
    seL4_CPtr ipcBufAddr;
    void* stackTop;
} ThreadConfig;



static int requestNewThreadConfig(seL4_CPtr endpoint, ThreadConfig* conf)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 7);
    seL4_SetMR(0, SyscallID_RequestCap);
    seL4_SetMR(1, SofaRequestCap_NewThread2);



    /*
    MR 1 status : 0 ok
       2 tcb
       3 ep
       4 ipcbuf
       5 ipcbufAddr
       6 stacktop
    */
    seL4_Call(endpoint, info);

    if(seL4_GetMR(1) != 0)
    {
        return seL4_GetMR(1);
    }

    conf->tcb = seL4_GetMR(2);
    conf->ep = seL4_GetMR(3);
    conf->ipcBuf = seL4_GetMR(4);
    conf->ipcBufAddr = seL4_GetMR(5);
    conf->stackTop = (void*) seL4_GetMR(6);

    return 0;
}

static void thRun(void *arg0, void *arg1, void *ipc_buf)
{   

    seL4_CPtr ep = (seL4_CPtr) arg0;
    assert(ep);
    assert(ipc_buf);

    //while(1);
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Exit);
    seL4_SetMR(1, 10);
    seL4_Send(ep, info);

    while(1);
}
static void startThread()
{
    ThreadConfig conf;
    requestNewThreadConfig(endpoint, &conf);
#if 1
    sel4utils_thread_t th;
    th.tcb.cptr = conf.tcb;
    th.stack_top = conf.stackTop;
    th.initial_stack_pointer = th.stack_top;
    th.stack_size = 1; // num pages
    th.ipc_buffer = conf.ipcBuf;
    th.ipc_buffer_addr = conf.ipcBufAddr;

    assert(th.ipc_buffer_addr % 512 == 0);
    assert(th.ipc_buffer);

    seL4_Word data = api_make_guard_skip_word(seL4_WordBits - TEST_PROCESS_CSPACE_SIZE_BITS);
    SFPrintf("->seL4_TCB_Configure using ipc cap %lu\n", th.ipc_buffer);
    seL4_Error err = seL4_TCB_Configure(th.tcb.cptr, conf.ep, init_data->root_cnode, data, init_data->vspace_root, 0, th.ipc_buffer_addr, th.ipc_buffer);
    SFPrintf("seL4_TCB_Configure:err=%i\n", err);


    err = seL4_TCB_SetPriority(th.tcb.cptr, init_data->tcb, 254);

    SFPrintf("set prio ret %i\n", err);

    seL4_DebugNameThread(th.tcb.cptr, "Thread");

    err = sel4utils_start_thread(&th, thRun, (void*) conf.ep, NULL, 1);

    SFPrintf("start thread ret %i\n", err);
#endif
#if 0
    seL4_CPtr tcb = conf.tcb;
//    seL4_CPtr ep = conf.ep;
    seL4_CPtr ipcBufCap = conf.ipcBuf;
    seL4_CPtr ipcBuff = conf.ipcBufAddr;
    seL4_CPtr stackTop = conf.stackTop;


    SFPrintf("TCB cap %ld\n", tcb);

    seL4_Word data = api_make_guard_skip_word(seL4_WordBits - TEST_PROCESS_CSPACE_SIZE_BITS);
    SFPrintf("Guard is %lX\n", data);




    seL4_Error err = seL4_TCB_Configure(tcb, conf.ep, init_data->root_cnode, data, init_data->vspace_root, 0, ipcBuff, ipcBufCap);
    SFPrintf("seL4_TCB_Configure:err=%i\n", err);


    assert(ipcBuff % 512 == 0);
    SFPrintf("IPC buffer %ld\n", ipcBuff);

    err = seL4_TCB_SetPriority(tcb, init_data->tcb, 254);
    SFPrintf("seL4_TCB_SetPriority:err=%i\n", err);

    seL4_UserContext regs = {0};
    err = seL4_TCB_ReadRegisters(tcb, 0, 0, sizeof(regs)/sizeof(seL4_Word), &regs);
    SFPrintf("seL4_TCB_ReadRegisters:err=%i\n", err);

    size_t context_size = sizeof(seL4_UserContext) / sizeof(seL4_Word);
    SFPrintf("Stack top %ld\n", stackTop);
    size_t tls_size = sel4runtime_get_tls_size();
    uintptr_t tls_base = (uintptr_t)stackTop - tls_size;
    uintptr_t tp = (uintptr_t)sel4runtime_write_tls_image((void *)tls_base);

//    seL4_IPCBuffer *ipc_buffer_addr = (void *)ipcBuff;
    sel4runtime_set_tls_variable(tp, __sel4_ipc_buffer, (seL4_IPCBuffer *) ipcBuff);

    uintptr_t aligned_stack_pointer = ALIGN_DOWN(tls_base, STACK_CALL_ALIGNMENT);

    err = sel4utils_arch_init_context(thRun, (void *) aligned_stack_pointer, &regs);
/*    
    err = sel4utils_arch_init_local_context(thRun, (void*)conf.ep, NULL,
                                                (void *) ipcBuff,
                                                (void *) aligned_stack_pointer,
                                                &regs);
*/
    assert(err == 0);

    err = seL4_TCB_WriteRegisters(tcb, 1, 0, sizeof(regs)/sizeof(seL4_Word), &regs);
    SFPrintf("seL4_TCB_WriteRegisters:err=%i\n", err);    

    seL4_DebugNameThread(tcb, "Thread");
/*
    err = seL4_TCB_SetIPCBuffer(tcb, ipcBuff, ipcBufCap);
    if(err != 0)
    {
        SFPrintf("seL4_TCB_SetIPCBuffer err=%i\n", err);
    }
    assert(err == 0);
*/
    err = seL4_TCB_Resume(tcb);
    SFPrintf("seL4_TCB_Resume:err=%i\n", err);
#endif
}




int main(int argc, char *argv[])
{
    endpoint = (seL4_CPtr) atoi(argv[0]);
    init_data = (test_init_data_t *) atol(argv[1]);
    RuntimeInit2(argc, argv);

    if(SFGetPid() != 1)
    {
        return EXIT_FAILURE;
    }

    
    if(ProcClientInit() !=0)
    {
        return EXIT_FAILURE;
    }

    SFPrintf("---- Userland unit tests ----\n");
    int unittestsPid = ProcClientSpawn("/cpio/utests");
    int utestStatus = -1;
    waitpid(unittestsPid, &utestStatus, 0);
    SFPrintf("Unit tests returned %i\n", utestStatus);
    SFPrintf("-----------------------------\n");

///////
    startThread();
//    ssize_t errOrCap = SFRegisterService("init");

////
    const char shellPath[] = "/cpio/shell";
    int shellPid = ProcClientSpawn(shellPath);
    SFPrintf("[init] shell pid is %i\n", shellPid);

    int appStatus = 0;

    while (1)
    {
        pid_t retPid = wait(&appStatus);
        SFPrintf("[init] Wait returned pid %i status %i\n", retPid, appStatus);
        if(retPid == shellPid)
        {
            shellPid = ProcClientSpawn(shellPath);
            SFPrintf("[init] shell pid is %i\n", shellPid);
        }
    }
    return 1;
}

