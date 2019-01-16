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

#include "KObject_Tests.h"
#include <assert.h>
#include <string.h>

#include "KObject/KObject.h"


static void releaseNeverCalled(struct kref *k)
{
    assert(0);
}

static int isCalled = 0;
static void releaseCalled(struct kref *k)
{
    assert(k);
    assert(k->refcount == 0);
    isCalled = 1;
}

void KObject_kref_Test()
{
    struct kref ret;
    kref_init(&ret);
    
    assert(ret.refcount == 1);
    
    
    kref_get(&ret);
    assert(ret.refcount == 2);
    
    kref_put(&ret, releaseNeverCalled);
    assert(ret.refcount == 1);
    
    assert(isCalled == 0);
    kref_put(&ret, releaseCalled);
    assert(isCalled == 1);
    assert(ret.refcount == 0);
    
}


void KObject_kobject_Test()
{
    struct kobject obj;
    
    kobject_init(&obj);
    assert(obj.parent == NULL);
    assert(obj.ktype == NULL);
    assert(obj.kref.refcount == 1);
    assert( kobject_name(&obj) == NULL);
    
    struct kobject *objRef = kobject_get(&obj);
    assert(objRef == &obj);
    assert(obj.kref.refcount == 2);
    
    kobject_put(objRef);
    assert(obj.kref.refcount == 1);
    
    kobject_put(objRef);
    assert(obj.kref.refcount == 0);
}


static char CustomName[] = "CustomName";
static struct kobj_type customType;
static void CustomRelease(struct kobject *object)
{
    assert(object->ktype == &customType);
    assert(object->kref.refcount == 0);
    assert(object->k_name && object->k_name == CustomName);
    
}

void KObject_customType()
{
    customType.release =  CustomRelease;
    
    struct kobject customObj;
    kobject_init(&customObj);
    assert(customObj.kref.refcount == 1);
    customObj.k_name = CustomName;
    customObj.ktype = &customType;
    
    kobject_put(&customObj);
    
}


void KSet_Tests()
{
    struct kset set;
    
    kset_init(&set);
    assert(kset_count(&set) == 0);
    
    struct kobject obj1;
    kobject_init(&obj1);
    assert(obj1.kref.refcount == 1);
    
    assert(kset_contains(&set, &obj1) == OSError_None);
    assert(kset_append(&set, &obj1) == OSError_None);
    
    assert(kset_count(&set) == 1);
    assert(obj1.kref.refcount == 2);
    
    assert(obj1.parent == &set.obj);
    assert(kset_contains(&set, &obj1)  );
    
    assert(kset_remove(&set, &obj1) == OSError_None );
    
    assert(obj1.parent == NULL );
    assert(obj1.kref.refcount == 1);
    assert(kset_contains(&set, &obj1) == 0);
    
    assert(kset_count(&set) == 0);
    
    
    assert( kset_getChildByName(&set, "") == NULL);
    
    assert( kset_getChildByName(&set, "prout") == NULL);
    
    
}

void KObject_AllTests()
{
    KObject_kref_Test();
    KObject_kobject_Test();
    KObject_customType();
    
    KSet_Tests();
}
