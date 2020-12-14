#include <cpio/cpio.h>
#include "SyscallTable.h"
#include "Process.h"
#include "utils.h"



extern char _cpio_archive[];
extern char _cpio_archive_end[];

static char** tokenizeArgs(const char *args, int* numSegs)
{
    char delim[] = " ";

    char* path = strdup(args);
    char *ptr = strtok(path, delim);
    int num = 0;

    char** segments = NULL;
    while(ptr != NULL)
	{
//		printf("%i '%s'\n", num, ptr);

        segments = realloc(segments, (num+1)*sizeof(char*));
        assert(segments);
        segments[num] = strdup(ptr);
        num++;
		ptr = strtok(NULL, delim);        
	}
    *numSegs = num;
    free(path);
    return segments;
}

void Syscall_spawn(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* process = caller->_base.process;

    char* dataBuf = caller->ipcBuffer;
    assert(dataBuf);

    if(strlen(dataBuf) == 0)
    {
        seL4_SetMR(1, -EINVAL);
        seL4_Reply(info);
        return;
    }

    int argc = 0;
    char** args = tokenizeArgs(dataBuf, &argc);
    assert(args);
    if(argc == 0)
    {
        seL4_SetMR(1, -EINVAL);
        seL4_Reply(info);
        return;
    }

    unsigned long fileSize = 0;
    void* fileLoc = cpio_get_file(_cpio_archive, _cpio_archive_end - _cpio_archive, args[0], &fileSize);

    if(fileLoc == NULL)
    {
        seL4_SetMR(1, -ENOENT);
        seL4_Reply(info);
        return;
    }

    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    newProc->argc = argc;
    newProc->argv = args;

    spawnApp(newProc, args[0], process);
    seL4_SetMR(1, newProc->init->pid);
    seL4_Reply(info);
}