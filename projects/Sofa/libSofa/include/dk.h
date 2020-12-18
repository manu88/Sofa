#pragma once

typedef enum
{
    DKRequest_Register,
    DKRequest_List,    
}DKRequest;

#define DeviceKitServiceName (const char*)"deviceKit"

int DKClientInit(void);

int DKClientEnum(void);