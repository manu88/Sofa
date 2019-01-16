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
#include "../Sofa.h"
#include "utlist.h"

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


struct kref
{
    atomic_t refcount;
};


void kref_init(struct kref* k) NO_NULL_POINTERS;
void kref_get(struct kref* k) NO_NULL_POINTERS;
void kref_put(struct kref* k,void (*release)(struct kref *k)) NO_NULL_ARGS(1,1);

/* ktype def */
struct kobject;

struct kobj_type
{
    void (*release)(struct kobject *);
};


/* kobject def */

#define KOBJ_NAME_LEN   20

struct kobject {
    char                    *k_name;
    //char                    name[KOBJ_NAME_LEN];
    struct kref             kref;
//    struct list_head        entry;
    struct kobject          *parent;
//    struct kset             *kset;
    struct kobj_type        *ktype;
//    struct dentry           *dentry;
    
    
    // when in a set
    struct kobject *prev, *next;
};

void kobject_init(struct kobject* object) NO_NULL_POINTERS;


struct kobject *kobject_get(struct kobject *ko) NO_NULL_POINTERS;
void kobject_put(struct kobject *ko) NO_NULL_POINTERS;

static inline NO_NULL_POINTERS const char* kobject_name(const struct kobject* obj) 
{
    return obj->k_name;
}

/* kset def */

struct kset
{
    struct kobject obj;
    struct kobject* _listHead;
};

void kset_init(struct kset* set) NO_NULL_POINTERS;

// Will take ownership, ie retain the kobject
OSError kset_append(struct kset*set , struct kobject* obj) NO_NULL_POINTERS;

// will release the kobject
OSError kset_remove(struct kset*set , struct kobject* obj) NO_NULL_POINTERS;

// returns 1 on success
int kset_contains(struct kset* set , struct kobject *obj) NO_NULL_POINTERS;

size_t  kset_count(const struct kset* set) NO_NULL_POINTERS;

struct kobject* kset_getChildByName( const struct kset* set , const char* name ) NO_NULL_POINTERS;

#define kset_foreach(set, el) DL_FOREACH(set->_listHead, el)



