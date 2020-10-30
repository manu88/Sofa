#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>


int main(int argc, char *argv[])
{
    int ret = ProcessInit(atoi(argv[1]));
    assert(ret == 0);

    printf("TimeServer started pid is %i ppid is %i\n", getpid(), getppid());

    seL4_CPtr timer_notif_cap = RequestCap(0);
    seL4_CPtr timer_irq_handler = RequestCap(1);
    assert(timer_notif_cap);
    assert(timer_irq_handler);

    while(1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t message = seL4_Recv(timer_notif_cap, &sender);
        printf("User Got response from %lu\n", sender);

        ret = seL4_IRQHandler_Ack(timer_irq_handler);
        assert(ret == 0);
    }
    
    return 0;
}

