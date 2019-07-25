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

#include "KObject.h"
#include <string.h>
#include <stdlib.h>

static void KobjectDefaultRelease(KObject *);

static KObjectMethods _defaultOBJMethods =
{
    KobjectDefaultRelease,
};

static void KOBJgetInfos( const KObject *obj , char outDesc[MAX_DESC_SIZE] );
static void KSETgetInfos( const KObject *obj , char outDesc[MAX_DESC_SIZE] );

static const KClass objClass =
KClassMake(
           "KObject",
           KOBJgetInfos,
           NULL // release
           );

const KClass* KObjectClass = &objClass;


static const KClass setClass =
KClassMake(
           "KSet",
           KSETgetInfos,
           NULL // release
           );

const KClass* KSetClass = &setClass;

void kref_init(struct kref* k)
{
    k->refcount = 1;
}

void kref_get(struct kref* k)
{
    atomic_fetch_add(&k->refcount, 1);
}

void kref_put(struct kref* k)
{
    atomic_fetch_sub(&k->refcount, 1);

}




void KObjectInit(KObject* object)
{
    memset(object, 0, sizeof(KObject) );
    kref_init(&object->kref);
    object->_class = &objClass;
    object->methods = _defaultOBJMethods;
}

void KObjectInitWithName(KObject* object, const char*name)
{
    KObjectInit(object);
    object->k_name = name;
}


KObject *KObjectGet(KObject *ko)
{
    kref_get(&ko->kref);
    return ko;
}
void KObjectPut(KObject *ko)
{
    kref_put(&ko->kref);
    
    if (ko->kref.refcount == 0)
    {
        ko->methods.release(ko);
        
        /*
        if( ko->class && ko->class->release)
        {
            ko->class->release(ko);
        }
        */
    }
}

static void KOBJgetInfos(const KObject *obj , char outDesc[MAX_DESC_SIZE] )
{
    //snprintf(outDesc, MAX_DESC_SIZE, "(refc %i)" ,  obj->kref.refcount );
}

static void KSETgetInfos(const KObject *obj , char outDesc[MAX_DESC_SIZE] )
{
    KSet *self = (KSet *) obj;
    snprintf(outDesc, MAX_DESC_SIZE, "%zi children ",   KSetCount(self)  );
}


void KSetInit(KSet* set)
{
    KObjectInit(&set->obj);
    set->obj._class = &setClass;
    set->obj.isSet = 1;
    set->_listHead = NULL;
}

void KSetInitWithName(KSet* set , const char* name)
{
    KSetInit(set);
    set->obj.k_name = name;
}

int KSetAppend(KSet*set , KObject* obj)
{
    DL_APPEND(set->_listHead, obj);
    KObjectGet(obj); // inc ref count
    obj->_parent = &set->obj;

    return 0;
}

static int PtrCmp(KObject *obj1 , KObject *obj2)
{
    return !(obj1 == obj2);
}

int KSetContains(KSet* set , KObject *obj)
{
    // (head,out,elt,cmp)
    KObject* out = NULL;
    DL_SEARCH(set->_listHead , out , out , PtrCmp);
    return out != NULL;
}

int KSetRemove(KSet*set , KObject* obj)
{
    DL_DELETE(set->_listHead, obj);
    obj->_parent = NULL;
    KObjectPut(obj); // dec ref count
    return 0;
}

size_t  KSetCount(const KSet* set)
{
    size_t c = 0;
    KObject* iter = NULL;
    DL_COUNT(set->_listHead, iter, c);

    return c;
}


KObject* KSetGetChildByName( const KSet* set , const char* name )
{
    KObject* obj = NULL;
    KSetForeach( set, obj)
    {
        if ( obj &&  strcmp( obj->k_name  , name) == 0)
        {
            return obj;
        }
    }
    
    return NULL;
}



static void _printOBJ( const KObject* obj , int indent)
{
    ALWAYS_ASSERT(obj);
    for(int i =0;i<indent;i++)
        kprintf("|\t");
    
    char desc[MAX_DESC_SIZE] = "";
    
    ALWAYS_ASSERT(obj->_class);
    obj->_class->getInfos(obj , desc);
    
    kprintf("'%s' %s %p refc %i %s\n" , obj->k_name, obj->_class->name, (const void*)obj, obj->kref.refcount, desc);
    
    if (KObjectIsSet(obj))
    {
        const KSet* set = (const KSet* ) obj;
        
        KObject* child = NULL;
        KSetForeach(set, child)
        {
            _printOBJ(child, indent + 1);
        }
    }
}


void KObjectPrintTree( const KObject* obj)
{
    _printOBJ(obj, 0);
}


KObject* KObjectResolve( const char* path_ , KSet* startNode )
{
    if (strlen(path_) == 0)
        return NULL;
    
    
    char* path = strdup(path_);
    ALWAYS_ASSERT(path);
    static const char delim[] = "/";
    
    char* token = strtok(path, delim);
    
    KSet *ret = startNode;
    
    while ( token != NULL )
    {
        if (KObjectIsSet((const KObject *)ret))
        {
            ret = (KSet *) KSetGetChildByName(ret, token);// InodeGetChildByName(ret ,token);
        }
        else
        {
            ret = NULL;
        }
        if(!ret)
        {
            free(path);
            return NULL;
        }
        token = strtok(NULL, delim);
        
    }
    /*
     char *last = strrchr(path, '/');
     if (last)
     {
     printf("Got a last '%s' \n" , last);
     }
     */
    
    free(path);
    return  (KObject*)ret;
}


static void KobjectDefaultRelease(KObject *obj)
{
    UNUSED_PARAMETER(obj);
}
