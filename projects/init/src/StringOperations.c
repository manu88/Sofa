//
//  StringOperations.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include "StringOperations.h"


uint32_t StringHash(const char *str)
{
    uint32_t hash = 5381;
    uint32_t c;
    
    while ((c = (uint32_t)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}
