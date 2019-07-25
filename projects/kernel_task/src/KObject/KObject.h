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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "../Sofa.h"
#include "utlist.h"


SOFA_BEGIN_DCL

#ifdef __APPLE__
    #include <stdatomic.h>
    typedef atomic_int atomic_t;
#else 
    typedef int atomic_t;

    #define atomic_fetch_sub(object, value) ({ atomic_t v = *object; *object-=1; v;})
    #define atomic_fetch_add(object, value) ({ atomic_t v = *object; *object+=1; v;})

#endif


#define container_of(ptr, type, member) ({                      \
const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
(type *)( (char *)__mptr - offsetof(type,member) );})

//!  kref
/*!
 A ref counted object
 */
struct kref
{
    atomic_t refcount;
};

/**
 Initialize an instance of kref
 @param k an instance of kref that can't be NULL
 */
void kref_init(struct kref* k) NO_NULL_POINTERS;

/**
 Increases the ref count of an instance of kref.
 @param k an instance of kref that can't be NULL
 */
void kref_get(struct kref* k) NO_NULL_POINTERS;

/**
 decreases the ref count of an instance of kref.
 @param k an instance of kref that can't be NULL
 */
void kref_put(struct kref* k) NO_NULL_POINTERS;

/* ktype def */
struct _KObject; // forward

typedef void (*KobjectRelease)(struct _KObject *);

#define MAX_DESC_SIZE 64

//!  KClass
/*!
 Represents an object `class`. Each KObject has a direct reference to a KClass instance. See KObjectIsKindOf.
 Note that there is no notion of inheritance chain in this system. If an object C 'inherits' from B that 'inherits' from A, C will have no way to know about A.
 
 */
typedef struct _KClass
{
    
    const char* name;
    
    void (*getInfos)(const struct _KObject *obj , char outDesc[MAX_DESC_SIZE] );
    KobjectRelease release;
} KClass;

/**
 Creates a new KClass instance.
 @param name the class name
 @param getInfos a function to get informations about a certain instance
 @param release a release function thas is deprecated
 */
#define KClassMake( name , getInfos,release ) { name  , getInfos,release}


/*!
 the base class of every KObject.
 */
extern const KClass* KObjectClass;

/*!
 the KSet class
 */
extern const KClass* KSetClass;

/* KObject def */

typedef struct
{
    KobjectRelease release;
    
}KObjectMethods;


//!  KObject
/*!
 The base class of every managed objects.
 */
typedef struct _KObject
{
    const char                    *k_name;
    struct kref                  kref;

    struct _KObject          *_parent;

    KObjectMethods methods;
    
    
    uint8_t isSet:1;
    const KClass * _class;
    
    // when in a set
    struct _KObject *prev, *next;
} KObject;

void KObjectInit(KObject* object) NO_NULL_POINTERS;
void KObjectInitWithName(KObject* object, const char*name) NO_NULL_POINTERS;

KObject *KObjectGet(KObject *ko) NO_NULL_POINTERS;
void KObjectPut(KObject *ko) NO_NULL_POINTERS;


static inline NO_NULL_POINTERS int KObjectIsKindOf( const KObject* o , const KClass* _class )
{
    return o->_class == _class;
}

static inline NO_NULL_POINTERS const char* KObjectGetName(const KObject* obj)
{
    return obj->k_name;
}

static inline NO_NULL_POINTERS KObject* KObjectGetParent( const KObject* obj)
{
    return obj->_parent;
}


static inline NO_NULL_POINTERS uint8_t KObjectIsSet( const KObject* obj)
{
    return obj->isSet;
}

//!  KSet
/*!
 The base class of every object that own an other object.
 */
typedef struct _KSet
{
    KObject obj;
    KObject* _listHead;
} KSet;


void KSetInit(KSet* set) NO_NULL_POINTERS;
void KSetInitWithName(KSet*set , const char* name) NO_NULL_POINTERS;

// Will take ownership, ie retain the KObject
int KSetAppend(KSet*set , KObject* obj) NO_NULL_POINTERS;

// will release the KObject
int KSetRemove(KSet*set , KObject* obj) NO_NULL_POINTERS;

// returns 1 on success
int KSetContains(KSet* set , KObject *obj) NO_NULL_POINTERS;

size_t  KSetCount(const KSet* set) NO_NULL_POINTERS;


KObject* KSetGetChildByName( const KSet* set , const char* name ) NO_NULL_POINTERS;

#define KSetForeach( set, el) DL_FOREACH(((set)->_listHead), el)



void KObjectPrintTree( const KObject* obj) NO_NULL_POINTERS;

KObject* KObjectResolve( const char* path_ , KSet* startNode ) NO_NULL_POINTERS;


SOFA_END_DCL
