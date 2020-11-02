#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>
#include <sys_calls.h>


static seL4_CPtr timer_notif_cap = 0;
static seL4_CPtr timer_irq_handler = 0;

static void* inter_thread(void*arg)
{
    printf("[TimerServer] thread is running\n");
    timer_notif_cap = RequestCap(0);


    while (1)
    {
        /* code */
    }

    
    timer_irq_handler = RequestCap(1);
    assert(timer_notif_cap);
    assert(timer_irq_handler);

    while(1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t message = seL4_Recv(timer_notif_cap, &sender);
        printf("User Got response from %lu\n", sender);

        int ret = seL4_IRQHandler_Ack(timer_irq_handler);
        assert(ret == 0);
    }

    
}
int main(int argc, char *argv[])
{
    int ret = ProcessInit(atoi(argv[1]));
    assert(ret == 0);

    printf("[TimeServer] started pid is %i ppid is %i\n", getpid(), getppid());
    pthread_t th;
    ret =  pthread_create(&th, NULL, inter_thread, NULL);

    while (1)
    {
    }
    
    seL4_CPtr ep = registerIPCService(TIME_SERVER_NAME, seL4_AllRights);
    //printf("[TimeServer] Sent cap\n");


    
    seL4_Word sender;
    while (1)
    {  
        seL4_MessageInfo_t m = seL4_Recv(ep, &sender);
        const TimeServerSysCall rpcID = (TimeServerSysCall) seL4_GetMR(0);

        if(rpcID == TimeServerSysCall_Sleep)
        {
            time_t sec = seL4_GetMR(1);
            time_t nsec = seL4_GetMR(2);

            printf("[TimeServer] request to sleep %li sec %li nsec msg from %lu!\n",sec, nsec, sender);

            seL4_SetMR(0, 42);
            seL4_Reply(m);
        }


    }


    
    return 0;
}

