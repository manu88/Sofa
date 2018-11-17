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

#include "IODevice.h"
#include"Bootstrap.h"
#include "Sofa.h"




int DriverKitInit(KernelTaskContext* context);

int DriverKitRegisterDevice( seL4_Word badge,  IOBaseDevice* device) SOFA_UNIT_TESTABLE NO_NULL_POINTERS ;

int DriverKitRemoveDevice( IOBaseDevice* device) SOFA_UNIT_TESTABLE NO_NULL_POINTERS ;


IOBaseDevice* DriverKitGetDeviceForBadge( seL4_Word badge) SOFA_UNIT_TESTABLE NO_NULL_POINTERS;
