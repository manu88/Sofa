#pragma once


#include "Bootstrap.h"
#include "DevServer.h"

DeviceOperations* EGADriverGetDeviceOps(void);

int InitEGADriver(InitContext *context);
