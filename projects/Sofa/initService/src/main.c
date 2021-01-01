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
#include <Sofa.h>
#include <Thread.h>
#include <runtime.h>
#include <proc.h>
#include <sys/wait.h>


#include <sel4utils/helpers.h>
#include <sel4runtime.h>


int main(int argc, char *argv[])
{
    RuntimeInit2(argc, argv);

    SFPrintf("Init: create service\n");

    ssize_t errOrCap = SFRegisterService("init");
    if(errOrCap <= 0)
    {
        SFPrintf("Error: unable to register init service\n");
        return -1;
    }

    void* p = NULL;
    while(1)
    {
        seL4_Word sender;
        seL4_MessageInfo_t msg = seL4_Recv(errOrCap, &sender);

        long code = seL4_GetMR(0);
        SFPrintf("Init service: received msg from %lu code %i\n", sender, code);

        if(code == 1)
        {
            p = SFMmap(NULL, 4096, 0, 0, 0, 0);
            SFPrintf("Init service: shared buff init side = %p\n", p);
            long addr = SFShareMem(p, sender);

            seL4_SetMR(0, addr);
            seL4_Reply(msg);
        }
        else if(code == 2)
        {
            SFPrintf("Received msg '%s'\n", p);
        }
    }


}

