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

#include "../ProcessSysCall.h"
#include "../Bootstrap.h"
#include <platsupport/chardev.h>

void processWrite(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    printf("[%s] %s" ,ProcessGetName(sender), ProcessGetIPCBuffer(sender));
    seL4_SetMR(1 , 0); // no err
    Reply( info);
}

void processRead(Process *sender,seL4_MessageInfo_t info , seL4_Word sender_badge)
{
    unsigned long sizeToRead = seL4_GetMR(1);
    
    
    int c = ps_cdev_getchar(&getKernelTaskContext()->comDev);
    /*
     do
     {
     c = ps_cdev_getchar(&getKernelTaskContext()->comDev);
     }
     while (c<=0);
     */
    if( c == '\n' || c == '\r')
    {
        ps_cdev_putchar(&getKernelTaskContext()->comDev , '\n');
    }
    else if( c > 0)
    {
        ps_cdev_putchar(&getKernelTaskContext()->comDev , c);
    }
    
    ProcessGetIPCBuffer(sender)[0] = c;
    ProcessGetIPCBuffer(sender)[1] = 0;
    
    seL4_SetMR(0,SysCall_Read);
    seL4_SetMR(1, c >0? 1 : 0 );
    Reply( info );
}
