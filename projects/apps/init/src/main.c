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



    printf("init : Hello\n");

    return 0;
}
