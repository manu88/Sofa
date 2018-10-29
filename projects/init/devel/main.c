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


static Inode* CpioOpen(void* context, const char*pathname ,int flags)
{
    printf("CpioOpen '%s' flags %i\n" ,pathname , flags);
    return 0;
}

int main(int argc, const char * argv[])
{
    
    
    FileServerHandler cpioHandler;
    cpioHandler.perfix = "/cpio/";
    cpioHandler.onOpen = CpioOpen;
    
    FileServerInit();
    
    assert(FileServerRegisterHandler(&cpioHandler) );
    
    
    FileServerOpen(NULL, "/cpio/test", 0);
    
    assert(FileServerOpen(NULL, "/cpi/test", 0) == NULL);
    
    return 0;
}
