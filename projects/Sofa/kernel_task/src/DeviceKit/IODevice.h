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

typedef ssize_t (*ReadDevice)(struct _IODevice* dev, char* buf, size_t bufSize);
typedef ssize_t (*WriteDevice)(struct _IODevice* dev, const char* buf, size_t bufSize);

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

}IODevice;


#define IODeviceInit(name_, type_) {.name = name_ ,.type = type_}

