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
#include <Thread.h>
#include <runtime.h>
#include "syscalls.h"
#include "test_init_data.h"

extern test_init_data_t *init_data;


static void __entryPoint(void *arg0, void *arg1, void *ipc_buf)
{
    Thread* self = (Thread*) arg0;
    assert(self);

    TLSContext ctx = {.buffer = (uint8_t*) self->sofaIPC, .ep = self->ep};

    TLSSet(&ctx);
    int r = self->main(self, arg1);

    TLSSet(NULL);
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0, SyscallID_ThreadExit);
    seL4_SetMR(1, r);
    seL4_Send(self->ep, info);

    seL4_TCB_Suspend(self->th.tcb.cptr);

    while (1);    
}

int ThreadInit(Thread* t, ThreadMain threadMain, void* arg)
{
    ThreadConfig conf;
    int err = sc_newThread(TLSGet()->ep, &conf);

    if(err != 0)
    {
        return err;
    }


    t->th.tcb.cptr = conf.tcb;
    t->th.stack_top = conf.stackTop;
    t->th.initial_stack_pointer = t->th.stack_top;
    t->th.stack_size = 1; // num pages
    t->th.ipc_buffer = conf.ipcBuf;
    t->th.ipc_buffer_addr = conf.ipcBufAddr;

    assert(t->th.ipc_buffer_addr % 512 == 0);
    assert(t->th.ipc_buffer);

    seL4_Word data = api_make_guard_skip_word(seL4_WordBits - TEST_PROCESS_CSPACE_SIZE_BITS);
    SFPrintf("->seL4_TCB_Configure using ipc cap %lu\n", t->th.ipc_buffer);
    err = seL4_TCB_Configure(t->th.tcb.cptr, conf.ep, init_data->root_cnode, data, init_data->vspace_root, 0, t->th.ipc_buffer_addr, t->th.ipc_buffer);
    SFPrintf("seL4_TCB_Configure:err=%i\n", err);


    err = seL4_TCB_SetPriority(t->th.tcb.cptr, init_data->tcb, 254);

    SFPrintf("set prio ret %i\n", err);

    seL4_DebugNameThread(t->th.tcb.cptr, "Thread");

    t->ep = conf.ep;
    t->main = threadMain;
    t->sofaIPC = conf.sofaIPC;

    assert(t->sofaIPC);

    err = sel4utils_start_thread(&t->th, __entryPoint, t, arg, 1);

    SFPrintf("start thread ret %i\n", err);


    return 0;
}