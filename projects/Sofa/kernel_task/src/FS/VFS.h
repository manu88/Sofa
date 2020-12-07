#pragma once
#include "IODevice.h"


typedef enum
{
    VFSSupported_Unknown = 0,
    VFSSupported_EXT2,
}VFSSupported;

int VFSInit(void);


int VFSStart(void);

int VFSAddDEvice(IODevice *dev);


void VFSLs(const char* path);