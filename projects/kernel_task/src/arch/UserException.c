
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

/*
 X86 exception handler
 */
typedef enum
{
    ExceptionCode_DivByZero = 0,
    
}ExceptionCode;

void processUserException(Process *sender, seL4_MessageInfo_t message , seL4_Word sender_badge)
{
    //klog("[kernel_task] seL4_UserException from %i %s \n" , sender->pid,  ProcessGetName(sender) );
    
    ExceptionCode errNum = (uint8_t) seL4_GetMR(3);
    
    
    
    switch (errNum)
    {
        case ExceptionCode_DivByZero:
            ProcessKill(sender , SofaSignal_ArithmeticError);
            break;
            
        default:
        {
            int msgLen = seL4_MessageInfo_get_length(message);
            klog("[kernel_task] Unknown User Exception msg content (len=%i) \n" , msgLen);
            klog(" EIP = %p\n"
                 " ESP = %p\n"
                 " EFLAGS = %p\n"
                 " exception number = %x\n"
                 " exception code = %p\n",
                 (void*)seL4_GetMR(0),
                 (void*)seL4_GetMR(1),
                 (void*)seL4_GetMR(2),
                 errNum,
                 (void*)seL4_GetMR(4));
            
            assert(0);
            break;
        }
    }
    
}
