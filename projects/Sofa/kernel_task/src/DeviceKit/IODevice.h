#pragma once
#include "utlist.h"
#include <sys/types.h>


typedef enum
{
    IODevice_Unknown = 0,
    IODevice_Net,
    IODevice_BlockDev,

} IODeviceType;

struct _IODevice;

typedef ssize_t (*ReadDevice)(struct _IODevice* dev, size_t sector, char* buf, size_t bufSize);
typedef ssize_t (*WriteDevice)(struct _IODevice* dev, size_t sector, const char* buf, size_t bufSize);

typedef struct
{
     ReadDevice read;
     WriteDevice write;
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


#define IODeviceInit(name_, type_, ops_) {.name = name_ ,.type = type_, .ops = ops_ }

ssize_t IODeviceRead(IODevice* dev, size_t sector, char* buf, size_t bufSize);
ssize_t IODeviceWrite(IODevice* dev, size_t sector, const char* buf, size_t bufSize);