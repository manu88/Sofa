#include "SyscallTable.h"
#include "Process.h"
#include "utils.h"
#include "VFS.h"

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
    VFS_File_Stat stat;
    int st = VFSStat(args[0], &stat);
    if(st != 0)
    {
        seL4_SetMR(1, -st);
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