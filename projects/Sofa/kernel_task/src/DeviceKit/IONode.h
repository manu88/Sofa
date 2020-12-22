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
#include <string.h>
#include <sys/types.h>

typedef struct 
{
    union IOVariantValue  
    {
         uint64_t v;
         const char* s;  
    } value;

    enum IOVariantType
    {
        IOVariantType_UINT64,
        IOVariantType_STRING
    } type;
    
} IOVariant;


#define _IOValString(val) {.s=val}
#define _IOValUINT64(val) {.v=val}
#define IOValueString(val) {.value = _IOValString(val), .type= STRING}
#define IOValueUINT64(val) {.value = _IOValUINT64(val), .type= UINT64}


typedef struct _IODriver IODriver;

typedef struct _IONode
{
    struct _IONode * children;
    struct _IONode* next;
    char* name;

    IOVariant hid;

    IODriver* driver;
} IONode;


#define IONodeNew(name_) {.children = NULL, .next = NULL, .name = name_, .driver = NULL}

static inline void IONodeInit(IONode* node, const char* name)
{
    memset(node, 0, sizeof(IONode));
    node->name = name;
}

// returns -1 if not an EISA ID, 0 otherwise.
int IONodeGetEISAID(const IONode*n, char eisaID[9]);

int IONodeEISAIDIs(const IONode*n, const char* eisaID);

static inline void IONodeAddChild(IONode* node, IONode* child)
{
    LL_APPEND(node->children, child);
}

#define IONodeForEachChildren(node, iter) LL_FOREACH(node->children, iter)

static inline IONode* IONodeGetChildren(const IONode* n, const char*name)
{
    IONode* c = NULL;
    IONodeForEachChildren(n, c)
    {
        if(strcmp(c->name, name) == 0)
        {
            return c;
        }
    }
    return NULL;
}

static inline size_t IONodeCountChildren(const IONode* node)
{
    size_t c = 0;
    IONode* t = NULL;
    LL_COUNT(node->children, t, c);
    return c;
}
