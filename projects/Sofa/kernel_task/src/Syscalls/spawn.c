/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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