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
#include <stdio.h>

/*
#ifndef __APPLE__
#include <utils/list.h>
#else
#include "list.h"
#endif
 */
#include <assert.h>

#include "uthash.h"

#define HASH_ADD_PID(head,key,add)  HASH_ADD(hh,head,key,sizeof(pid_t),add)
#define HASH_FIND_PID(head,key,out) HASH_FIND(hh,head,key,sizeof(pid_t),out)



typedef struct
{
    Inode procNode;

    Process* _processes; // the process list


    FileOperations _procFolderFileOps;
    INodeOperations _inodeOps;
    
    
    pid_t accum;
    
} ProcTableContext;


static ssize_t ProcRead ( Inode * node, char*buffer  , size_t count);
//ssize_t ProcWrite ( Inode *node,  const char* buffer ,size_t count);

static FileOperations _processFileOps =
{
    ProcRead , FileOperation_NoWrite , FileOperation_NoLseek
};

//static ssize_t ProcFolderRead (Inode *node, char* buffer , size_t count);

static ProcTableContext _ctx;


pid_t ProcessTableGetNextPid()
{
    
    return _ctx.accum++;
}

static const char procName[]  = "proc";
static const char statusStr[] = "status";
static const char cmdlineStr[] = "cmdline";

int ProcessTableInit()
{
    memset(&_ctx, 0, sizeof(ProcTableContext));
    
//    _ctx._inodeOps.Open  = ProcFolderOpen;
//    _ctx._inodeOps.Close = ProcFolderClose;
    _ctx._procFolderFileOps.Read = ProcRead;

    
    if (InodeInit(&_ctx.procNode, INodeType_Folder, procName) == 0)
    {
        return 0;
    }
//    _ctx.procNode.operations = &_ctx._procFolderFileOps;
//    _ctx.procNode.inodeOperations = &_ctx._inodeOps;
    
    _ctx._processes = NULL;
    
    return 1;
    //return list_init(&_ctx._processes) == 0;
}

Inode* ProcessTableGetInode()
{
    return &_ctx.procNode;
}

int ProcessTableGetCount()
{
    return HASH_COUNT(_ctx._processes);
    //return list_length(&_ctx._processes);
}



int ProcessTableAppend( Process* process)
{
    if (ProcessTableContains(process))
    {
        return 0;
    }
    
    pid_t nextP = ProcessTableGetNextPid();
    
    process->_pid = nextP;// ProcessTableGetNextPid();
    
    HASH_ADD_PID(_ctx._processes, _pid, process);
    
	if(1)//list_append(&_ctx._processes , process) == 0)
    {
        
    
        char str[32];
        sprintf(str, "%i", process->_pid);
    
        char* nodeName = strdup(str);
        if (!InodeInit(&process->_processNode, INodeType_Folder, nodeName ))
        {
            free(nodeName);
            return 0;
        }
        process->_processNode.userData = process;
        
        if( !InodeAddChild(ProcessTableGetInode(), &process->_processNode))
        {
            return 0;
        }
        
        int err = 0;
        // create statusnode
        Inode* statusNode = InodeAlloc(INodeType_File , statusStr);
        assert(statusNode);
        statusNode->operations =&_processFileOps;
        statusNode->userData = process;
        statusNode->size = 1; // one status byte
        err = !InodeAddChild(&process->_processNode , statusNode);
        assert(err == 0);

        // create cmdline
        Inode* cmdlineNode = InodeAlloc(INodeType_File , cmdlineStr);
        assert(cmdlineNode);
        cmdlineNode->operations =&_processFileOps;
        cmdlineNode->userData = process;
        cmdlineNode->size = 0; // one status byte
        err = !InodeAddChild(&process->_processNode , cmdlineNode);
        assert(err == 0);
        return 1;
    }
    
    return 0;
}

int ProcessTableContains( const Process* process)
{
    assert(process);
    
    Process* p = NULL;
    
    HASH_FIND_PID(_ctx._processes, &process->_pid, p);
    
    if (p && p == process)
    {
        return 1;
    }
    return 0;
    /*
    
    list_t *l = &_ctx._processes;
    
    for (struct list_node *n = l->head; n != NULL; n = n->next)
    {
        Process* p = (Process*) n->data;
        
        if (p == process)
        {
            return 1;
        }
    }
    
    return 0;
     */
}

Process* ProcessTableGetByPID( pid_t pid)
{
    
    Process* p = NULL;
    
    HASH_FIND_PID(_ctx._processes, &pid, p);
    
    return p;
    /*
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
     */
}


static int PtrComp(void*a, void*b)
{
    return a != b;
}

int ProcessTableRemove(Process* process)
{
    if (ProcessTableContains(process))
    {
        HASH_DEL(_ctx._processes, process);
        return InodeRemoveChild( &_ctx.procNode, &process->_processNode);
    }
    return 0;
    /*
    if(list_remove(&_ctx._processes , process,PtrComp) == 0)
    {
        return InodeRemoveChild( &_ctx.procNode, &process->_processNode);
    }
    
    return 0;
     */
}



static ssize_t _CmdLineRead(Inode *node, char* buffer , size_t count)
{
	Process* process = node->userData;

	if (node->size == 0)
	{
		node->size = strlen(process->cmdLine);
	}

	assert( node->size);
//	printf("_CmdLineRead for process '%s'\n" , process->cmdLine);
	assert(process->cmdLine);
	const ssize_t remainSize = (ssize_t)(node->size - node->pos);
	assert(remainSize>=0);

	ssize_t toRead = count < remainSize ? (ssize_t)count : remainSize;

	memcpy(buffer , process->cmdLine + node->pos , toRead);

	node->pos += (size_t)toRead;
	return toRead;
/*
        if(node->pos == 0)
        {
		//buffer = node->userData
                node->pos = 1;
                return 1;
        }

        return 0;
*/
	return 0;
}
static ssize_t _StatusRead(Inode *node, char* buffer , size_t count)
{
	
	Process* process = node->userData;

        if(node->pos == 0)
        {
                sprintf(buffer, "%i", process->_state);
                node->pos = 1;
                return 1;
        }

        return 0;
}

static ssize_t ProcRead (Inode *node, char* buffer , size_t count)
{
//	printf("ProcRead node name '%s'\n" , node->name);


	if (strcmp(node->name , statusStr ) == 0)
	{
		return _StatusRead(node, buffer , count);
	}
	else if (strcmp(node->name , cmdlineStr) == 0)
	{
		return _CmdLineRead(node, buffer , count);
	}

/*
	Process* process = node->userData;

	if(node->pos == 0)
	{
		sprintf(buffer, "%i", process->_state);
		node->pos = 1;
		return 1;
	}
*/
	return 0;
}



int ProcessTableAddAndStart(KernelTaskContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority )
{
	int error = !ProcessTableAppend(process);
	assert(error == 0);

	error = !ProcessSetCmdLine(process , imageName);
	assert(error == 0);

	error = ProcessStart(context , process , imageName , ep_cap_path , parent , priority);

	assert(error == 0);

	return error;
}
