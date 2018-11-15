/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include "../SysCalls.h"
#include <SysCallNum.h>
#include <assert.h>
#include "../FileServer.h"
#include <fcntl.h>

int handle_kill(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
        seL4_Word pidToKill = seL4_GetMR(1);
    seL4_Word sigToSend = seL4_GetMR(2);
    
    printf("Received a request from %i to kill process %li with signal %li\n",senderProcess->_pid , pidToKill , sigToSend);
    seL4_SetMR(1, -ENOSYS ); // error for now
    seL4_Reply( message );

    return 0;
}
