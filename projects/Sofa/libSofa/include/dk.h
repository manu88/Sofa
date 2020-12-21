#pragma once

typedef enum
{
    DKRequest_Register,
    DKRequest_List,
    DKRequest_Tree,
}DKRequest;

#define DeviceKitServiceName (const char*)"deviceKit"

int DKClientInit(void);

int DKClientEnumDevices(void);
int DKClientEnumTree(void);