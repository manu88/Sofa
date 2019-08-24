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
//
//  Sofa.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 03/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#pragma once
#include <assert.h>
#include <stdio.h>


#ifdef __cplusplus
#define SOFA_BEGIN_DCL extern "C" {
#define SOFA_END_DCL }
#else
#define SOFA_BEGIN_DCL
#define SOFA_END_DCL
#endif

#define SOFA_VERSION_MAJ   0
#define SOFA_VERSION_MIN   13
#define SOFA_VERSION_PATCH 0

#define SOFA_DEPRECATED(version) __attribute((deprecated))

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

#ifndef NO_NULL_POINTERS
#define NO_NULL_POINTERS              __attribute__((nonnull))
#endif

#ifndef NO_NULL_ARGS
#define NO_NULL_ARGS(index, range)    __attribute__((nonnull(index, range) ))
#endif

#ifndef WARN_UNUSED_RESULT
#define WARN_UNUSED_RESULT            __attribute__((warn_unused_result))
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif


#ifndef NO_RETURN
#define NO_RETURN __attribute__((noreturn))
#endif

// Mark methods with no Sel4 dependencies that can be tested on other platforms
#define SOFA_UNIT_TESTABLE

#define ALWAYS_ASSERT(x) assert(x)

#define kprintf(args, ...) printf(args, ##__VA_ARGS__)
#define klog(args, ...) printf(args, ##__VA_ARGS__)
/*
typedef enum
{
    OSError_None                 = 0,
    OSError_Some                 = -1,
    
    OSError_ArgError             = -2,
    OSError_InitError            = -10,
    OSError_ParsingError         = -20,
    OSError_Unimplemented        = -100,
    OSError_NotSupportedDevice,
    OSError_NotSupportedFS,
    OSError_AlreadyInSet,
    OSError_DeviceNotFound,
    
} OSError;
*/

#define ALWAYS_ASSERT_NO_ERR(x) assert(x == OSError_None)


#define __packed     __attribute__((packed))

#define STATIC_ASSERT(predicate, file) _impl_CASSERT_LINE(predicate,__LINE__,file)

#define _impl_PASTE(a,b) a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
typedef char _impl_PASTE(assertion_failed_##file##_,line)[2*!!(predicate)-1];


void Panic( const char* message) NO_RETURN;
