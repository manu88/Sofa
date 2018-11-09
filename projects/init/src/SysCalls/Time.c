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
#include <time.h>
#include "../Timer.h"


int handle_getTimeOfTheDay(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	return 0;
}


int handle_clock_getTime(InitContext* context, Process *senderProcess, seL4_MessageInfo_t message)
{
	const clockid_t clk_id = seL4_GetMR(1);
	printf("Got clock_gettime for clock %i\n" , clk_id);

	seL4_Word ret = -EINVAL;


	switch(clk_id)
	{
		case CLOCK_MONOTONIC:
		
			ret = TimerGetTime(context);
		
		break;
		
		default:
		ret = -EINVAL;
	}

	message = seL4_MessageInfo_new(0, 0, 0, 2);
	seL4_SetMR(0, __SOFA_NR_clock_gettime );
	seL4_SetMR(1, ret );

	seL4_Reply( message );

	return 0;
}


