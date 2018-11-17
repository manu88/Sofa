#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SysClient.h>

#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>

int main( int argc , char* argv[])
{

    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }

    const char t[] = "Test Client\n";

    write(0 , t , strlen(t) );
    printf("Client : Hello\n");

/*
    int ret = usleep(1000*4000);
    assert(ret == 0);
    assert(errno == 0);


    printf("Client After sleep\n");
*/
    return 10;
}

