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

#include "Stats.h"
#include <stdio.h>
#include <stdint.h>

#include <sel4/benchmark_utilisation_types.h>
#include <sel4/sel4.h>


int TestStats()
{

//	uint64_t *ipcbuffer = (uint64_t*) &(seL4_GetIPCBuffer()->msg[0]);


	//seL4_BenchmarkGetThreadUtilisation(seL4_CapInitThreadTCB);
	//printf("TestStats\n");
	return 0;
}
