/*
 * This file is part of the Sofa project
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
#include "utlist.h"
#include "IONode.h"


int DeviceTreeInit(void);

int DeviceTreeX86Start(void);

IODevice* DeviceTreeGetDevices(void);

#define FOR_EACH_DEVICE(dev) DL_FOREACH(DeviceTreeGetDevices(), dev)

int DeviceTreeAddDevice( IODevice* dev);

typedef void (*PoweroffMethod)(void* ptr);

int DeviceTreeAddPoweroff(PoweroffMethod method, void* ptr);

const IONode* DeviceTreeGetRoot(void);

void DeviceTreePrint(const IONode* node);



void DeviceTreeHardwarePoweroff(void);