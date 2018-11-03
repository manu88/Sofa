//
//  FileServer.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#pragma once

#include <stdio.h>
#include "fs.h"
#include "Sofa.h"



#ifdef __APPLE__
typedef void InitContext;
#else 
#include "Bootstrap.h"
#endif

/* File System handler definition */
typedef Inode* (* FileServerHandler_Open) (void* context, const char*pathname ,int flags, int *error) ;



typedef struct
{
    const char* perfix;
    FileServerHandler_Open onOpen;
    
} FileServerHandler;


int FileServerInit(void) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

int FileServerRegisterHandler( FileServerHandler* handler) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;



Inode* FileServerOpen(InitContext* context , const char*pathname , int flags , int *error) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;


