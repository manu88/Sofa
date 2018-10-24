#include "ProcessDef.h"


#include "ProcessTable.h"

Process* ProcessAlloc()
{
    Process* p = malloc(sizeof(Process));

    ProcessInit(p);
    return p;
}

int ProcessInit(Process* process)
{
    memset(process , 0 , sizeof(Process) );
    return 1;
}


int ProcessRelease(Process* process)
{
    free(process);
    return 1;
}


int startProcess(InitContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority )
{
    UNUSED int error = 0;

    sel4utils_process_config_t config = process_config_default_simple( &context->simple, imageName, priority);

    error = sel4utils_configure_process_custom( &process->_process , &context->vka , &context->vspace, config);

    ZF_LOGF_IFERR(error, "Failed to spawn a new thread.\n"
                  "\tsel4utils_configure_process expands an ELF file into our VSpace.\n"
                  "\tBe sure you've properly configured a VSpace manager using sel4utils_bootstrap_vspace_with_bootinfo.\n"
                  "\tBe sure you've passed the correct component name for the new thread!\n");



    process->_pid = getNextPid();

    seL4_CPtr process_ep_cap = 0;

    process_ep_cap = sel4utils_mint_cap_to_process(&process->_process, ep_cap_path,
                                               seL4_AllRights, process->_pid);

    ZF_LOGF_IF(process_ep_cap == 0, "Failed to mint a badged copy of the IPC endpoint into the new thread's CSpace.\n"
               "\tsel4utils_mint_cap_to_process takes a cspacepath_t: double check what you passed.\n");


    process_config_fault_cptr(config, ep_cap_path.capPtr);

    seL4_Word argc = 1;
    char string_args[argc][WORD_STRING_SIZE];
    char* argv[argc];
    sel4utils_create_word_args(string_args, argv, argc ,process_ep_cap);


    printf("init : Start child \n");
    error = sel4utils_spawn_process_v(&process->_process , &context->vka , &context->vspace , argc, (char**) &argv , 1);
    ZF_LOGF_IFERR(error, "Failed to spawn and start the new thread.\n"
                  "\tVerify: the new thread is being executed in the root thread's VSpace.\n"
                  "\tIn this case, the CSpaces are different, but the VSpaces are the same.\n"
                  "\tDouble check your vspace_t argument.\n");

     printf("init : Did start child pid %i\n" , process->_pid);


     process->_parent = parent;
     return error;
}
