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
#include "../utils.h"
#include "Timer.h"
#include "Log.h"
#include <platsupport/time_manager.h>
#include <Sofa.h>

static int sleep_callback(uintptr_t token)
{
    Thread* caller = (Thread*) token;
    assert(caller);

    seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
    seL4_SetMR(0, SyscallID_Sleep);
    seL4_SetMR(1, 0); // sucess

    seL4_Send(caller->_base.replyCap, tag);

    ThreadCleanupTimer(caller);
    return 0;
}


void Syscall_sleep(Thread* caller, seL4_MessageInfo_t info)
{
    KernelTaskContext* env = getKernelTaskContext();
    assert(caller->_base.replyCap == 0);

    unsigned int timerID = 0;
    int error = tm_alloc_id(&env->tm, &timerID);
    if (error)
    {
        KLOG_TRACE("Unable to alloc timer id err=%i\n", error);

        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }


    seL4_Word slot = get_free_slot(&env->vka);
    error = cnode_savecaller(&env->vka, slot);
    if (error)
    {
        KLOG_TRACE("Unable to save caller err=%i\n", error);
        cnode_delete(&env->vka, slot);
        tm_free_id(&env->tm, timerID);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }

    error = tm_register_cb(&env->tm, TIMEOUT_RELATIVE, seL4_GetMR(1) * NS_IN_MS, 0, timerID , sleep_callback, (uintptr_t) caller);
    if(error)
    {
        KLOG_TRACE("tm_register_failed, err=%i\n", error);
        tm_free_id(&env->tm, timerID);
        cnode_delete(&env->vka, slot);
        seL4_SetMR(1, -error);
        seL4_Reply(info);
        return;
    }

    caller->_base.currentSyscallID = SyscallID_Sleep;
    caller->_base.timerID = timerID;
    caller->_base.replyCap = slot;
}

void Syscall_GetTime(Thread* caller, seL4_MessageInfo_t info)
{
    union
    {
        uint64_t v;
        uint32_t s[2];
    } value;
    
    value.v =  GetTime();

    seL4_SetMR(1, value.s[0]);
    seL4_SetMR(2, value.s[1]);

    seL4_Reply(info);
}