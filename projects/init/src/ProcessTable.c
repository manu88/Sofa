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

typedef struct
{
    Inode procNode;
    list_t _processes;


    FileOperations _procFolderFileOps;
    INodeOperations _inodeOps;
} ProcTableContext;


static ssize_t ProcRead ( Inode * node, char*buffer  , size_t count);
//ssize_t ProcWrite ( Inode *node,  const char* buffer ,size_t count);

static FileOperations _processFileOps =
{
    ProcRead , FileOperation_NoWrite , FileOperation_NoLseek
};

static ssize_t ProcFolderRead (Inode *node, char* buffer , size_t count);

static ProcTableContext _ctx;


pid_t ProcessTableGetNextPid()
{
    static pid_t accum = 1; // 1 is reserved for init
    return accum++;
}


int ProcessTableInit()
{
//    _ctx._inodeOps.Open  = ProcFolderOpen;
//    _ctx._inodeOps.Close = ProcFolderClose;
    _ctx._procFolderFileOps.Read = ProcRead;

    
    if (InodeInit(&_ctx.procNode, INodeType_Folder, "proc") == 0)
    {
        return 0;
    }
//    _ctx.procNode.operations = &_ctx._procFolderFileOps;
//    _ctx.procNode.inodeOperations = &_ctx._inodeOps;

    return list_init(&_ctx._processes) == 0;
}

Inode* ProcessTableGetInode()
{
    return &_ctx.procNode;
}

int ProcessTableGetCount()
{
    return list_length(&_ctx._processes);
}

static const char statusStr[] = "status";
int ProcessTableAppend( Process* process)
{
	if(list_append(&_ctx._processes , process) == 0)
    {
    
        process->_pid = ProcessTableGetNextPid();
    
        char str[32];
        sprintf(str, "%i", process->_pid);
    
        if (!InodeInit(&process->_processNode, INodeType_Folder, strdup(str) ))
        {
            return 0;
        }

	Inode* statusNode = InodeAlloc(INodeType_File , statusStr);
	assert(statusNode);
        statusNode->operations =&_processFileOps;
	statusNode->userData = process;

	InodeAddChild(&process->_processNode , statusNode);
        process->_processNode.userData = process;
    
        if( !InodeAddChild(ProcessTableGetInode(), &process->_processNode))
        {
            return 0;
        }
        
        return 1;
    }
    
    return 0;
}

Process* ProcessTableGetByPID( pid_t pid)
{
    list_t *l = &_ctx._processes;

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
    return list_remove(&_ctx._processes , process,PtrComp) == 0;
}


static ssize_t ProcRead (Inode *node, char* buffer , size_t count)
{
	Process* process = node->userData;
	printf("Proc read request for %i\n" , process->_pid);

	sprintf(buffer, "%i", process->_state);
	return 1;
}

