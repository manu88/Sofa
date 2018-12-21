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
//  Init_UnitTests.h
//  DevelSofaInit
//
//  Created by Manuel Deneu on 03/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include <assert.h>
#include "Sofa.h"
#include "Credentials.h"
int Identity_UnitTests()
{
    Identity id1;
    id1.uid = 1234;
    
    Identity id2;
    
    assert( IdentityCopy(&id1, &id2));
    
    assert(id1.uid == id2.uid);
    
    return 1;
}
