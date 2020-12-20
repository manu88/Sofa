#pragma once

#include "IODevice.h"
#include "utlist.h"



int DeviceTreeInit(void);

int DeviceTreeX86Start(void);

IODevice* DeviceTreeGetDevices(void);

#define FOR_EACH_DEVICE(dev) DL_FOREACH(DeviceTreeGetDevices(), dev)

int DeviceTreeAddDevice( IODevice* dev);