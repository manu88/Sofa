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
#include "proc.h"

#include "runtime.h"
#include <Sofa.h>
#include <stdarg.h>

const char procServiceName[] = "Proc";

seL4_CPtr procCap = 0;
char* procBuf = NULL;

int ProcClientInit()
{
    ssize_t capOrErr = SFGetService(procServiceName);

    if(capOrErr > 0)
    {
        procCap = capOrErr;

        seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
        seL4_SetMR(0, ProcRequest_Register);
        seL4_Call(procCap, info);
        procBuf = (char*) seL4_GetMR(1);
        return 0;
    }
    return -1;
}

int ProcClientEnum(OnProcessDescription callb, void* ptr)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, ProcRequest_Enum);
    seL4_Call(procCap, info);

    size_t numProc = seL4_GetMR(1);

    char* buff = procBuf;
    for(size_t i=0;i<numProc; i++)
    {
        const ProcessDesc* desc = (ProcessDesc*) buff; 

        int r = callb(desc, ptr);
        if( r!= 0)
        {
            return r;
        }

        size_t recSize = sizeof(ProcessDesc) + desc->nameLen;

        buff += recSize;
    }
}