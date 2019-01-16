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

#include "Sofa.h"
#include "KObject/KObject.h"


OSError DriverKitInit(void) SOFA_UNIT_TESTABLE;

OSError DriverKitContructDeviceTree( const uint8_t* fromDatas, size_t bufferSize) NO_NULL_ARGS(1, 1);

struct kobject* DriverKitGetObject(void) SOFA_UNIT_TESTABLE;


OSError DriverKitRegisterDevice( struct kobject* obj) NO_NULL_POINTERS SOFA_UNIT_TESTABLE;

void DriverKitDump(void);
