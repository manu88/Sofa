#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <Sofa.h>
#include <Thread.h>


void* thread1(void* arg)
{
    printf("Hello thread!\n");
    while (1)
    {
        /* code */
    }
    
}

int main(int argc, char *argv[])
{
    int ret = ProcessInit((void*) atoi(argv[1]));
    assert(ret == 0);

    printf("App: Hello world\n");

    pthread_t thread;

    pthread_create(&thread, NULL, thread1, NULL);

    while (1)
    {

    }
    
    return 0;
}

