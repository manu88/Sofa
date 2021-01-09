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



int main(int argc, char *argv[])
{
    RuntimeInit2(argc, argv);

    if(SFGetPid() != 1)
    {
        return EXIT_FAILURE;
    }

    
    if(ProcClientInit() !=0)
    {
        return EXIT_FAILURE;
    }

    SFPrintf("---- Userland unit tests ----\n");
    int unittestsPid = ProcClientSpawn("/cpio/utests");
    int utestStatus = -1;
    waitpid(unittestsPid, &utestStatus, 0);
    SFPrintf("Unit tests returned %i\n", utestStatus);
    SFPrintf("-----------------------------\n");


    const char initServicePath[] = "/cpio/initService";
    int initServPid = ProcClientSpawn(initServicePath);
    SFPrintf("[init] init service pid is %i\n", initServPid);


    const char shellPath[] = "/cpio/shell";
    int shellPid = ProcClientSpawn(shellPath);
    SFPrintf("[init] shell pid is %i\n", shellPid);

    int appStatus = 0;

    while (1)
    {
        pid_t retPid = wait(&appStatus);
        SFPrintf("[init] Wait returned pid %i status %i\n", retPid, appStatus);
        if(retPid == shellPid)
        {
            shellPid = ProcClientSpawn(shellPath);
            SFPrintf("[init] shell pid is %i\n", shellPid);
        }
    }
    return 1;
}

