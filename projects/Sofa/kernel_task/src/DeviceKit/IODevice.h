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
#include "utlist.h"
#include <sel4/types.h>
#include <sys/types.h>
#include <string.h>

typedef struct
{
    char* buff;
    size_t expectedSize;
    seL4_CPtr replyCap;
    size_t sector;
} IODeviceRequest;

void IODeviceRequestReply(IODeviceRequest* req, ssize_t ret);

typedef enum
{
    IODevice_Unknown = 0,
    IODevice_Net,
    IODevice_BlockDev,
    IODevice_CharDev,
    IODevice_Keyboard,
    IODevice_FrameBuffer,

// Not real Device types, can't be used for setting type, notably in IODeviceNew or IODeviceInit
    IODevice_AllTypes,
    IODevice_Last // keep last!

} IODeviceType;

struct _IODevice;
struct _Thread;

typedef ssize_t (*ReadDevice)(struct _IODevice* dev, size_t sector, char* buf, size_t bufSize);
typedef ssize_t (*WriteDevice)(struct _IODevice* dev, size_t sector, const char* buf, size_t bufSize);

typedef ssize_t (*ReadDeviceAsync)(struct _IODevice* dev,IODeviceRequest* reply);

typedef void (*HandleIRQ)(struct _IODevice* dev, int irqN);


typedef int (*RegisterIface)(struct _IODevice* dev, void* netInterface, const /*ip_addr_t*/void* addr, const /*ip_addr_t*/void*gw, const /*ip_addr_t*/void* mask);

typedef long (*MapMemory)(struct _IODevice* dev, struct _Thread* caller, int code);
typedef struct
{
    ReadDevice read; // called if asyncRead == 0
    ReadDeviceAsync readAsync; // called if asyncRead != 0

    WriteDevice write;
    HandleIRQ handleIRQ;
    RegisterIface regIface;
    MapMemory mapMemory;

    uint8_t asyncRead;
} IODeviceOperations;


typedef struct _IODevice
{
    char* name;

    IODeviceType type;

    struct _IODevice *next;
    struct _IODevice *prev;

    IODeviceOperations* ops;

    void* impl;

}IODevice;

#define IODeviceNew(name_, type_, ops_) {.name = name_ ,.type = type_, .ops = ops_ }

static inline void IODeviceInit(IODevice* d, char* name, IODeviceType type, IODeviceOperations* ops)
{
    memset(d, 0, sizeof(IODevice));
    d->name = name;
    d->type = type;
    d->ops = ops;
}

ssize_t IODeviceRead(IODevice* dev, size_t sector, char* buf, size_t bufSize);
ssize_t IODeviceWrite(IODevice* dev, size_t sector, const char* buf, size_t bufSize);

ssize_t IODeviceReadAsync(IODevice* dev, IODeviceRequest* reply);
