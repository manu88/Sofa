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


#pragma once


#include "Sofa.h"
#include "ProcessDef.h"
#include "fs.h"


int ProcessTableInit(void) SOFA_UNIT_TESTABLE;

Inode* ProcessTableGetInode(void) SOFA_UNIT_TESTABLE;

// wrapper that do Append and ProcessStart
int ProcessTableAddAndStart(KernelTaskContext* context, Process* process,const char* imageName, cspacepath_t ep_cap_path , Process* parent, uint8_t priority )NO_NULL_POINTERS;

int ProcessTableAppend( Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

// ineficient method that will 'for each' the process table
int ProcessTableContains( const Process* process) SOFA_UNIT_TESTABLE;

// ineficient method that will 'for each' the process table
Process* ProcessTableGetByPID( pid_t pid) SOFA_UNIT_TESTABLE;

int ProcessTableRemove(Process* process) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

int ProcessTableGetCount(void) SOFA_UNIT_TESTABLE;

pid_t ProcessTableGetNextPid(void);
