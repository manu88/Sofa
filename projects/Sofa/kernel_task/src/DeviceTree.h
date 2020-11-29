#pragma once
#include "utlist.h"


typedef enum
{
    IODevice_Unknown = 0,
    IODevice_Net,

} IODeviceType;

typedef struct _IODevice
{
    char* name;

    IODeviceType type;

    struct _IODevice *next;
    struct _IODevice *prev;

} IODevice;


#define IODeviceInit(name_, type_) {.name = name_ ,.type = type_}


int DeviceTreeInit(void);


IODevice* DeviceTreeGetDevices(void);

#define FOR_EACH_DEVICE(dev) DL_FOREACH(DeviceTreeGetDevices(), dev)

int DeviceTreeAddDevice( IODevice* dev);