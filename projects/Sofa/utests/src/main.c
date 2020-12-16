#include "runtime.h"
#include <Sofa.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
    
    RuntimeInit2(argc, argv);
    VFSClientInit();

    SFPrintf("Unit tests started\n");
    assert(0);

    int h = open("/fake/file1", O_RDONLY);
    assert(h != -1);

    char b[10] = "";
    ssize_t ret = read(h, b, 10);
    while (ret != EOF)
    {
        SFPrintf("read returned %zi '%s'\n", ret, b);

        ret = read(h, b, 10);
        if(ret > 0)
            b[ret] = 0;
    }
    close(h);
    
    

    //void *t = dlopen("Some file", RTLD_NOW);

    
    return 0;
}

