/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <sel4/types.h>


typedef enum
{
    // to keep sync'd with IODeviceType!
    DKDevice_Unknown = 0,
    DKDevice_Net,
    DKDevice_BlockDev,
    DKDevice_CharDev,
    DKDevice_FrameBuffer,

    DKDevice_AllTypes,
} DKDeviceType;

typedef enum
{
    DKRequest_Register,
    DKRequest_Tree,
    DKRequest_Enum,

    DKRequest_DeviceDetails,

    DKRequest_Read,
    DKRequest_Write,
    DKRequest_MMap,
}DKRequest;


typedef enum
{
    DKDeviceDetail_GetName,
    DKDeviceDetail_GetDevFile,
} DKDeviceDetails;

#define DeviceKitServiceName (const char*)"deviceKit"

int DKClientInit(void);


int DKClientEnumTree(void);

typedef seL4_Word DKDeviceHandle;
#define DKDeviceHandle_Invalid 0


typedef struct
{
    size_t count;
    DKDeviceHandle handles[];
}DKDeviceList;


// pass list = NULL to only count devices
int DKClientEnumDevices(DKDeviceType type, DKDeviceList* list, size_t* numDevices);



/* Per device operations*/
char* DKDeviceGetName(DKDeviceHandle devHandle);
char* DKDeviceGetDevFile(DKDeviceHandle devHandle);

ssize_t DKDeviceRead(DKDeviceHandle handle, size_t index, char* buf, size_t bufSize);
ssize_t DKDeviceWrite(DKDeviceHandle handle, size_t index, const char* buf, size_t bufSize);
long DKDeviceMMap(DKDeviceHandle handle, int code);

DKDeviceHandle DKClientGetDeviceNamed(const char* deviceName, DKDeviceType type);


