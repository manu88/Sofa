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
#include "../Utils.h"
#include "../ProcessTable.h"
#include "../MainLoop.h" // UpdateTimeout
#include <SysCallNum.h>
#include "../Timer.h"

typedef struct
{
	KernelTaskContext* context;
	Process *senderProcess;
	seL4_CPtr reply;

} ReplyContext;


int afterSleep(uintptr_t token)
{
	ReplyContext* ctx = (ReplyContext*) token;
	assert(ctx);
	assert(ctx->context);
	assert(ctx->senderProcess);
	assert(ctx->reply);

	int err = tm_deregister_cb(&ctx->context->tm  , ctx->senderProcess->_pid);
        assert(err == 0);

	err = tm_free_id(&ctx->context->tm , ctx->senderProcess->_pid);
	assert(err == 0);

	seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 2);
        seL4_SetMR(0, __SOFA_NR_nanosleep);
        seL4_SetMR(1, 0); // sucess

        seL4_Send(ctx->reply , tag);

        cnode_delete(ctx->context,ctx->reply);
}

int handle_nanosleep(KernelTaskContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
    int error = -ENOSYS;

    seL4_Word millisToWait = seL4_GetMR(1);
    
    assert(millisToWait > 0); // special 0 value must have been handled client side!

    ReplyContext* timerCtx = (ReplyContext*) malloc(sizeof(ReplyContext) );

    assert(timerCtx);
    timerCtx->context = context;
    timerCtx->senderProcess = senderProcess;
    timerCtx->reply = get_free_slot(context);
    error = cnode_savecaller( context, timerCtx->reply );
    assert(error == 0);

    error = TimerAllocAndRegisterOneShot(&context->tm , millisToWait * NS_IN_MS  , senderProcess->_pid , afterSleep , (uintptr_t) timerCtx );
    assert(!error);

#if 0
    TimerContext* timerCtx =(TimerContext*) malloc(sizeof(TimerContext));

    if(timerCtx)
    {

	if( TimerInit(&timerCtx->timer, timerCtx, 1/*Oneshot*/))
	{
		timerCtx->process = senderProcess;
		if(TimerWheelAddTimer(&context->timersWheel , &timerCtx->timer , millisToWait))
		{
			timerCtx->reply = get_free_slot(context);
			error = cnode_savecaller( context, timerCtx->reply );
			if(error == 0)
			{
				UpdateTimeout(context,millisToWait * NS_IN_MS);
				return 0;
			}
			else 
			{
				// TODO handle release of timerCtx->reply
				error = -ENOMEM;
				TimerWheelRemoveTimer(&context->timersWheel , &timerCtx->timer);
				free(timerCtx);
			}
		}
		else
		{
			free(timerCtx);
			error = -ENOMEM;
		}
	}
	else // TimerInit
	{
		free(timerCtx);
		error = -ENOMEM;
	}
    }
    else  // timerCtx malloc
    {
	error = -ENOMEM;
    }

    // sends a reponse on error
    if(error != 0)
    {
	seL4_SetMR(1,error);
	seL4_Reply( message );
    }
    
    // save the caller
    /*
    timerCtx->reply = get_free_slot(context);
    int error = cnode_savecaller( context, timerCtx->reply );
    assert(error == 0);

    assert(TimerInit(&timerCtx->timer, timerCtx, 1/));
    timerCtx->process = senderProcess;
    assert(TimerWheelAddTimer(&context->timersWheel , &timerCtx->timer , millisToWait));
    UpdateTimeout(context,millisToWait * NS_IN_MS);
	*/

#endif
    return 0;
}

