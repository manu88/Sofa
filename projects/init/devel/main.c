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



static Inode* CpioOpen(void* context, const char*pathname ,int flags, int *error)
{
    printf("CpioOpen '%s' flags %i\n" ,pathname , flags);
    return NULL;
}

int main(int argc, const char * argv[])
{
    
    
    FileServerHandler cpioHandler;
    cpioHandler.perfix = "/cpio/";
    cpioHandler.onOpen = CpioOpen;
    
    FileServerInit();
    
    assert(FileServerRegisterHandler(&cpioHandler) );
    
    int err = 0;
    FileServerOpen(NULL, "/cpio/test", 0 , &err);
    
    assert(FileServerOpen(NULL, "/cpi/test", 0 ,&err) == NULL);
    
    return 0;
}
