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

typedef enum
{
    DKRequest_Register,
    DKRequest_List,
    DKRequest_Tree,
    DKRequest_Enum,
}DKRequest;

#define DeviceKitServiceName (const char*)"deviceKit"

int DKClientInit(void);

int DKClientTempListDevices(void);
int DKClientEnumTree(void);

typedef seL4_Word DKDeviceHandle;

int DKClientEnumDevices(int type, size_t* numDevices);