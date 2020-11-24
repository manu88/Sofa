#include <cpio/cpio.h>
#include "SyscallTable.h"
#include "../testtypes.h"
#include "../utils.h"



extern char _cpio_archive[];
extern char _cpio_archive_end[];

void Syscall_spawn(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    Process* process = caller->process;

    const char* dataBuf = caller->ipcBuffer;

    unsigned long fileSize = 0;
    void* fileLoc = cpio_get_file(_cpio_archive, _cpio_archive_end - _cpio_archive, dataBuf, &fileSize);

    if(fileLoc == NULL)
    {
        seL4_SetMR(1, -ENOENT);
        seL4_Reply(info);
        return;
    }
    printf("File '%s' size %lu\n", dataBuf, fileSize);
    Process* newProc = kmalloc(sizeof(Process));
    ProcessInit(newProc);
    spawnApp(newProc, dataBuf, process);

    seL4_SetMR(1, newProc->init->pid);
    seL4_Reply(info);
}