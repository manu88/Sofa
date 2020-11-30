#pragma once

#include "IODevice.h"



int DeviceTreeInit(void);


IODevice* DeviceTreeGetDevices(void);

#define FOR_EACH_DEVICE(dev) DL_FOREACH(DeviceTreeGetDevices(), dev)

int DeviceTreeAddDevice( IODevice* dev);