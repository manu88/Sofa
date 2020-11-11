#include <allocman/vka.h>
#include <allocman/bootstrap.h>


#include "helpers.h"
#include "runtime.h"


static seL4_CPtr badge_endpoint(env_t env, seL4_Word badge, seL4_CPtr ep)
{
    seL4_CPtr slot = get_free_slot(env);
    int error = cnode_mint(env, ep, slot, seL4_AllRights, badge);
    assert(error == seL4_NoError);
    return slot;
}


static int thread1(seL4_Word ep, seL4_Word ep2, seL4_Word runs, seL4_Word arg3)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 44);
    seL4_Send(getProcessEndpoint(), info);
    return 0;
    int acc = 10;
    while(acc--)
    {
        seL4_MessageInfo_t info = seL4_MessageInfo_new(0, 0, 0, 1);
        seL4_SetMR(0, acc);
        seL4_Send((seL4_CPtr) ep, info);
    }
}




int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);

    seL4_CPtr sync_ep = vka_alloc_endpoint_leaky(&getProcessEnv()->vka);
    seL4_CPtr badged_sync_ep = badge_endpoint(getProcessEnv(), 10, sync_ep);
    helper_thread_t sync;
    create_helper_thread(getProcessEnv(), &sync);
    start_helper(getProcessEnv(), &sync, thread1, badged_sync_ep, getProcessEndpoint(), 0, 0);

    wait_for_helper(&sync);
    printf("Thread returned\n");

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 42);
    seL4_Send(getProcessEndpoint(), info);

    while (1)
    {
        /* code */
    }

}

