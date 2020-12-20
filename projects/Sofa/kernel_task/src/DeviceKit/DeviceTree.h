#pragma once

#include "IODevice.h"
#include "utlist.h"
typedef struct _IONode
{
    struct _IONode * children;

    struct _IONode* next;

    char* name;
} IONode;


#define IONodeNew(name_) {.children = NULL, .next = NULL, .name = name_}

static inline void IONodeInit(IONode* node, const char* name)
{
    memset(node, 0, sizeof(IONode));
    node->name = name;
}

static inline void IONodeAddChild(IONode* node, IONode* child)
{
    LL_APPEND(node->children, child);
}

static inline IONode* IONodeGetChildren(const IONode* n, const char*name)
{
    IONode* c = NULL;
    LL_FOREACH(n->children, c)
    {
        if(strcmp(c->name, name) == 0)
        {
            return c;
        }
    }
    return NULL;
}

int DeviceTreeInit(void);

int DeviceTreeX86Start(void);

IODevice* DeviceTreeGetDevices(void);

#define FOR_EACH_DEVICE(dev) DL_FOREACH(DeviceTreeGetDevices(), dev)

int DeviceTreeAddDevice( IODevice* dev);