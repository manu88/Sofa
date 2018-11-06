//
//  IODevice.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 05/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <string.h>
#include "IODevice.h"


int IOBaseDeviceInit(IOBaseDevice* device)
{
    memset(device, 0, sizeof(IOBaseDevice) );
    return 1;
}
