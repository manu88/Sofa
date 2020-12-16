#include "runtime.h"
#include <Sofa.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

static void testMmap()
{
    char* ptr = mmap(NULL, 4096, 0, 0,-1, 0);
    assert(ptr);
    ptr[0] = 1;
    ptr[4095] = 2;
    int ret = munmap(ptr, 4096);
    assert(ret == 0);
}

static void testMalloc(void)
{
    void *ptr = malloc(512);
    assert(ptr);
    free(ptr);
}
static void testRead(void)
{
    int h = open("/fake/file1", O_RDONLY);
    assert(h != -1);

    char b[10] = "";
    ssize_t ret = read(h, b, 10);
    while (ret != EOF)
    {
        ret = read(h, b, 10);
        if(ret > 0)
            b[ret] = 0;
    }
    close(h);
}

int main(int argc, char *argv[])
{
    
    RuntimeInit2(argc, argv);
    VFSClientInit();

    testRead();
    testMmap();
    
    return 0;
}

