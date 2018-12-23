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

#include <sel4/types.h>
#include "uthash.h"
#include "Sofa.h"

typedef enum
{
    IOBaseDeviceType_Unknown = 0,
    IOBaseDeviceType_PCI          = 100,
} IOBaseDeviceType;

struct _IOBaseDevice
{
    seL4_Word _badge;
    IOBaseDeviceType type;
    UT_hash_handle hh;
    
    int (*InitDevice) (struct _IOBaseDevice *device);
    int (*DeInitDevice) (struct _IOBaseDevice *device);
    
    int (*HandleIRQ) (struct _IOBaseDevice *device, int irqNum);
};

typedef struct _IOBaseDevice IOBaseDevice;


typedef struct
{
    IOBaseDevice base;
    
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_id;
    
} IODevicePCI;


// Will mostly set every fields to 0/NULL
int IOBaseDeviceInit(IOBaseDevice* device) SOFA_UNIT_TESTABLE NO_NULL_POINTERS;

