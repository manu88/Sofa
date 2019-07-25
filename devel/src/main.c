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

#include <stdio.h>
#include <stdlib.h>
#include <Process.h>




int main(int argc, const char * argv[])
{
    ProcessListInit();
    
    Process *p1 = malloc(sizeof(Process));
    ProcessInit(p1);
    
    vka_object_t fromEp;
    fromEp.cptr = 1;
    ProcessStart(p1, "init", &fromEp, NULL);
    KObjectPut((KObject *)p1);
    
    
    Process *p2 = malloc(sizeof(Process));
    ProcessInit(p2);
    
    ProcessStart(p2, "driverkitd", &fromEp, p1);
    KObjectPut((KObject *)p2);
    Process *p3 = malloc(sizeof(Process));
    ProcessInit( p3);
    
    ProcessStart(p3, "shell", &fromEp, p1);
    KObjectPut((KObject *)p3);
    
    
    
    Process *p4 = malloc(sizeof(Process));
    ProcessInit(p4);

    
    ProcessStart(p4, "test", &fromEp, p3);
    KObjectPut((KObject *)p4);
    
    ProcessDump();
    
    printf("--> kill 4\n");
    ProcessKill(p4);
    ProcessCleanup(p4);
    
    ProcessDump();
    
    assert(ProcessGetByPID(4) == NULL);
        
    return 0;
}
