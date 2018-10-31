//
//  main.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <stdio.h>
#include "FileServer.h"
#include <assert.h>
#include <errno.h>

#include <data_struct/chash.h>

#include "DevServer.h"


static int called = 0;

static Inode* CpioOpen(void* context, const char*pathname ,int flags, int *error)
{
    printf("CpioOpen '%s' flags %i\n" ,pathname , flags);
    
    called = 1;
    return NULL;
}

int main(int argc, const char * argv[])
{
    
    
    FileServerHandler cpioHandler;
    cpioHandler.perfix = "/cpio/";
    cpioHandler.onOpen = CpioOpen;
    
    
    DevServerInit();
    
    assert(FileServerInit() );
    
    assert(FileServerRegisterHandler(&cpioHandler) );
    assert(FileServerRegisterHandler(getDevServerHandler() ) );
    
    int err = 0;
    assert(FileServerOpen(NULL, "/cpio/test", 0 , &err) == NULL );
    
    assert(called == 1);
    
    called = 0;
    assert(FileServerOpen(NULL, "/cpi/test", 0 ,&err) == NULL);
    assert(called == 0);
    
    assert(FileServerOpen(NULL, "/dev/console", 0 ,&err) == NULL);
    
    return 0;
}
