#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>
#include <Spawn.h>


static void* inter_thread(void*arg)
{
    printf("[App-thread] started\n");

}
int main(int argc, char *argv[])
{
    int ret = ProcessInit(atoi(argv[1]));
    assert(ret == 0);

    for (int i=0;i<10;i++)
    {
        vka_object_t res;
        vka_alloc_endpoint(&getProcessContext()->vka, &res);
        printf("%i - %lu\n", i, res.cptr);
        assert(res.cptr);
    }

    printf("App started parent pid is %i\n", getppid());


    pthread_t th;
    ret =  pthread_create(&th, NULL, inter_thread, NULL);
    assert(ret == 0);
    
    printf("Start Thread join\n");
//    pthread_join(th, NULL);
//    printf("Thread join ok\n");

    while (1)
    {
    }
    

    seL4_CPtr cap = 0;
    while (cap ==  0)
    {
        cap = getIPCService("TimeServer.main");
    }
    
    printf("[App] received cap\n");


    tempSetTimeServerEP(cap);
    struct timespec tm;
    tm.tv_sec = 2;
    tm.tv_nsec = 0;

    nanosleep(&tm, NULL);

    while (1)
    {

    }
    
    return 0;
}

