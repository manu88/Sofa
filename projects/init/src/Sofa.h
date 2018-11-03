//
//  Sofa.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 03/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#ifndef Sofa_h
#define Sofa_h

#define SOFA_VERSION_MAJ   0
#define SOFA_VERSION_MIN   1
#define SOFA_VERSION_PATCH 0

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

#define SOFA_UNIT_TESTABLE



#endif /* Sofa_h */
