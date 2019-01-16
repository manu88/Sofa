/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include "SystemTree.h"
#include <stdio.h>
#include <assert.h>


static void _SystemTreeRelease(struct kobject *obj);

typedef struct
{
    struct kset obj; // NEEDS to stay first!
    
} SystemTreeRoot;

static struct _SystemTree
{
    SystemTreeRoot root;
    
} _systemTree;



static struct kobj_type _SystemRootType =
{
    _SystemTreeRelease, // release
};


static char SysRootName[] = "SysRoot";

OSError SystemTreeInit()
{
    kset_init(&_systemTree.root.obj);
    _systemTree.root.obj.obj.ktype = &_SystemRootType;
    _systemTree.root.obj.obj.k_name = SysRootName;
    
    // assign root's parent to root
    _systemTree.root.obj.obj.parent = &_systemTree.root.obj.obj;
    
    return OSError_None;
}

static void _SystemTreeRelease(struct kobject *obj)
{
    // THIS SHOULD NEVER BE CALLED
    assert(0);
}

struct kobject* SystemTreeGetRoot()
{
    return &_systemTree.root.obj.obj;
}


OSError SystemTreeAddObject( struct kobject *obj)
{
    return kset_append(&_systemTree.root.obj, obj);
}




static void _printObject(const struct kobject* object , int indent)
{
    
    
    for(int i =0;i<indent;i++)
        printf("\t");
    
    printf("'%s' \n" , object->k_name );//, child->type == INodeType_Folder? "Folder":"File");
    
    struct kobject *child = NULL;
    kset_foreach( ((struct kset*)object), child)
    {
        _printObject(child, indent +1);
        /*
        if (child->type == INodeType_Folder)
        {
            _printNode(child , indent + 1);
        }
         */
    }
}




void SystemTreeDump()
{
    printf("--- Start System Tree ---\n");
    
    _printObject( (const struct kobject*)&_systemTree.root.obj , 0);
    /*
    struct kobject *child = NULL;
    
    kset_foreach( (&_systemTree.root.obj), child)
    {
        printf(" -'%s'\n" , child->k_name);
    }
     */
    printf("--- End System Tree ---\n");
}
