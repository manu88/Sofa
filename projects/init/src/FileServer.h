//
//  FileServer.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#ifndef FileServer_h
#define FileServer_h

#include <stdio.h>



#ifdef __APPLE__
typedef void InitContext;
#else 
#include "Bootstrap.h"
#endif

/* File System handler definition */
typedef int (* FileServerHandler_Open) (void* context, const char*pathname ,int flags) ;

typedef struct
{
    const char* perfix;
    
    FileServerHandler_Open onOpen;
    
} FileServerHandler;





int FileServerInit(void);

int FileServerRegisterHandler( FileServerHandler* handler);



int FileServerOpen(InitContext* context , const char*pathname , int flags);

#endif /* FileServer_h */
