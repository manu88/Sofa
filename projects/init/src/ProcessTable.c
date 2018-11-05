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

#include "ProcessTable.h"


#ifndef __APPLE__
#include <utils/list.h>
#else
#include "list.h"
#endif


static list_t _processes;


pid_t ProcessTableGetNextPid()
{
    static pid_t accum = 2; // 1 is reserved for init
    return accum++;
}


int ProcessTableInit()
{ 
    return list_init(&_processes) == 0;
}


int ProcessTableGetCount()
{
    return list_length(&_processes);
}

int ProcessTableAppend( Process* process)
{
	return list_append(&_processes , process) == 0;
}

Process* ProcessTableGetByPID( pid_t pid)
{
    list_t *l = &_processes;

    for (struct list_node *n = l->head; n != NULL; n = n->next) 
    {
        Process* p = (Process*) n->data;
        
        if(p->_pid == pid)
        {
            return p;
        }
    }

	return NULL;
}


static int PtrComp(void*a, void*b)
{
    return a != b;
}

int ProcessTableRemove(Process* process)
{
    return list_remove(&_processes , process,PtrComp) == 0;
}



