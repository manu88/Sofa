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


static int on_thread1(seL4_Word ep, seL4_Word ep2, seL4_Word runs, seL4_Word arg3)
{
    printf("Hello thread1\n");
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

static int on_thread2(seL4_Word ep, seL4_Word ep2, seL4_Word runs, seL4_Word arg3)
{
    printf("Hello thread2\n");
    while(1);
}


int main(int argc, char *argv[])
{
    RuntimeInit(argc, argv);

    seL4_CPtr sync_ep = vka_alloc_endpoint_leaky(&getProcessEnv()->vka);
    seL4_CPtr badged_sync_ep = badge_endpoint(getProcessEnv(), 10, sync_ep);
    helper_thread_t thread1;
    create_helper_thread(getProcessEnv(), &thread1);
    start_helper(getProcessEnv(), &thread1, on_thread1, badged_sync_ep, getProcessEndpoint(), 0, 0);


    helper_thread_t thread2;
    create_helper_thread(getProcessEnv(), &thread2);
    start_helper(getProcessEnv(), &thread2, on_thread2, badged_sync_ep, getProcessEndpoint(), 0, 0);

    wait_for_helper(&thread1);
    printf("thread1 returned\n");

    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 42);
    seL4_Send(getProcessEndpoint(), info);

    while (1)
    {
        /* code */
    }

}

