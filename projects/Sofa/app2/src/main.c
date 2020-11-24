#include "runtime.h"
#include <Sofa.h>
#include <Thread.h>
#include <sel4utils/helpers.h>
#include <sel4runtime.h>

test_init_data_t *init_data;

#define TEST_PROCESS_CSPACE_SIZE_BITS 17


static seL4_CPtr requestcap(seL4_CPtr endpoint, SofaRequestCap capType)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_RequestCap);
    seL4_SetMR(1, capType);
    seL4_Call(endpoint, info);

    seL4_CPtr tcb = seL4_GetCap(0);
    return seL4_GetMR(1);
}


static void thRun(void *arg0, void *arg1, void *ipc_buf)
{
    //seL4_Word ud = seL4_GetUserData();
    //assert(ud);
    assert((int) arg0 == 1);
    while(1);
}

int main(int argc, char *argv[])
{
    seL4_CPtr endpoint = (seL4_CPtr) atoi(argv[0]);
    init_data = (test_init_data_t *) atol(argv[1]);
    
    RuntimeInit2(argc, argv);

    SofaPrintf("cnode cap %ld\n", init_data->root_cnode);
    SofaPrintf("vspace root cap %ld\n", init_data->vspace_root);

    seL4_CPtr tcb = requestcap(endpoint, SofaRequestCap_TCB);
    SofaPrintf("TCB cap %ld\n", tcb);

    seL4_Word data = api_make_guard_skip_word(seL4_WordBits - TEST_PROCESS_CSPACE_SIZE_BITS);
    SofaPrintf("Guard is %lX\n", data);

    seL4_CPtr faultEP = endpoint;
    seL4_Error err = seL4_TCB_Configure(tcb, faultEP, init_data->root_cnode, data, init_data->vspace_root, 0, 0, seL4_CapNull);
    SofaPrintf("seL4_TCB_Configure:err=%i\n", err);

    seL4_CPtr ipcBuff = requestcap(endpoint, SofaRequestCap_IPCBuff);
    SofaPrintf("IPC buffer %ld\n", ipcBuff);

    err = seL4_TCB_SetPriority(tcb, init_data->tcb, 254);
    SofaPrintf("seL4_TCB_SetPriority:err=%i\n", err);

    seL4_UserContext regs = {0};
    err = seL4_TCB_ReadRegisters(tcb, 0, 0, sizeof(regs)/sizeof(seL4_Word), &regs);
    SofaPrintf("seL4_TCB_ReadRegisters:err=%i\n", err);

    size_t context_size = sizeof(seL4_UserContext) / sizeof(seL4_Word);
    seL4_CPtr stackTop = requestcap(endpoint, SofaRequestCap_MAP);

    size_t tls_size = sel4runtime_get_tls_size();
    uintptr_t tls_base = (uintptr_t)stackTop - tls_size;
    uintptr_t tp = (uintptr_t)sel4runtime_write_tls_image((void *)tls_base);
    seL4_IPCBuffer *ipc_buffer_addr = (void *)ipcBuff;
    sel4runtime_set_tls_variable(tp, __sel4_ipc_buffer, ipc_buffer_addr);

    uintptr_t aligned_stack_pointer = ALIGN_DOWN(tls_base, STACK_CALL_ALIGNMENT);



    //sel4utils_arch_init_local_context(thRun, (void*)1, (void*)2, (void*)3, stackTop, &regs);
    err = sel4utils_arch_init_local_context(thRun, (void*)1, (void*)2,
                                                (void *) ipc_buffer_addr,
                                                (void *) aligned_stack_pointer,
                                                &regs);
    assert(err == 0);


    err = seL4_TCB_WriteRegisters(tcb, 1, 0, sizeof(regs)/sizeof(seL4_Word), &regs);
    SofaPrintf("seL4_TCB_WriteRegisters:err=%i\n", err);    
//    err = seL4_TCB_Resume(tcb);
//    printf("seL4_TCB_Resume:err=%i\n", err);

    err = seL4_TCB_Resume(tcb);
    SofaPrintf("seL4_TCB_Resume:err=%i\n", err);        
    SofaDebug(SofaDebugCode_DumpSched);
    while (1)
    {}
    
    return 1;
}

