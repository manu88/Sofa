#include <stdio.h>
#include "VFS.h"

int VFSInit()
{
    return 0;
}


int VFSAddDEvice(IODevice *dev)
{
    printf("[VFS] add device '%s'\n", dev->name);
    return 0;
}