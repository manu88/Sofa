//
//  main.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 29/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <unistd.h>

#include <stdio.h>
#include "FileServer.h"
#include <assert.h>
#include <errno.h>

#include <data_struct/chash.h>

#include "DevServer.h"

#include "Init_UnitTests.h"

static int cpioCalled = 0;
static Inode* CpioOpen(void* context, const char*pathname ,int flags, int *error)
{
    printf("CpioOpen '%s' flags %i\n" ,pathname , flags);
    
    cpioCalled = 1;
    
    return NULL;
}

static int consoleOpenCalled = 0;
static Inode* ConsoleOpen (struct _DeviceOperations * device, int flags )
{
    consoleOpenCalled = 1;
    Inode* node = malloc(sizeof(Inode) );
    node->operations = &device->fileOps;
    return node;
}


static ssize_t ConsoleWrite (struct _inode *node,  const char*buffer ,size_t size)
{
    printf("ConsolesWrite '%s' %zi \n" , buffer , size);
    
    return (ssize_t)size;
}

int main(int argc, const char * argv[])
{
    doInit_UnitTests();
    
    FileServerHandler cpioHandler;
    cpioHandler.perfix = "/cpio/";
    cpioHandler.onOpen = CpioOpen;
    
    assert(DevServerInit() );
    
    assert(FileServerInit() );
    
    assert(FileServerRegisterHandler(&cpioHandler) );
    assert(FileServerRegisterHandler(&cpioHandler)  == 0); // second time must fail
    
    assert(FileServerRegisterHandler(getDevServerHandler() ) );
    assert(FileServerRegisterHandler(getDevServerHandler() ) == 0 ); // second time must fail
    
    DeviceOperations ops;
    ops.OpenDevice = ConsoleOpen;
    ops.fileOps.Write = ConsoleWrite;
    
    assert(DevServerRegisterFile("/lol", &ops) == 0);
    assert(DevServerRegisterFile("/lol", NULL) == 0);
    assert(DevServerRegisterFile("console", &ops) );
    
    int err = 0;
    int fakeContextSoThatClangIsHappy = 1;
    assert(FileServerOpen(&fakeContextSoThatClangIsHappy, "/cpio/test", 0 , &err) == NULL );
    
    assert(cpioCalled == 1);
    
    cpioCalled = 0;
    assert(FileServerOpen(&fakeContextSoThatClangIsHappy, "/cpi/test", 0 ,&err) == NULL);
    assert(cpioCalled == 0);
    
    Inode* consoleNode = FileServerOpen(&fakeContextSoThatClangIsHappy, "/dev/console", 0 ,&err);
    assert( consoleNode != NULL);
    assert(consoleOpenCalled == 1);
    
    consoleNode->operations->Write(consoleNode ,"hello" ,4);
    free(consoleNode);
    
    //sleep(4);
    return 0;
}
