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
//  StringOperations.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 31/10/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <string.h>
#include "StringOperations.h"


uint32_t StringHash(const char *str)
{
    uint32_t hash = 5381;
    uint32_t c;
    
    while ((c = (uint32_t)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}


void StringPrepend(char* s, const char* t)
{
    size_t len = strlen(t);
    size_t i;
    
    memmove(s + len, s, strlen(s) + 1);
    
    for (i = 0; i < len; ++i)
    {
        s[i] = t[i];
    }
}
