#pragma once
#include "IODevice.h"


typedef enum
{
    VFSSupported_Unknown = 0,
    VFSSupported_EXT2,
}VFSSupported;

int VFSServiceInit(void);


int VFSServiceStart(void);

//int VFSAddDEvice(IODevice *dev);

