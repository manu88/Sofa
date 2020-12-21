#pragma once

#include "IODevice.h"
#include "utlist.h"
#include "IONode.h"


int DeviceTreeInit(void);

int DeviceTreeX86Start(void);

IODevice* DeviceTreeGetDevices(void);

#define FOR_EACH_DEVICE(dev) DL_FOREACH(DeviceTreeGetDevices(), dev)

int DeviceTreeAddDevice( IODevice* dev);

typedef void (*PoweroffMethod)(void* ptr);

int DeviceTreeAddPoweroff(PoweroffMethod method, void* ptr);

const IONode* DeviceTreeGetRoot(void);

void DeviceTreePrint(const IONode* node);



void DeviceTreeHardwarePoweroff(void);