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
#include <stddef.h>
#include <stdint.h>
#include "IODevice.h"

OSError DeviceTreeContructDeviceTree(IODevice* root, const uint8_t* fromDatas, size_t bufferSize) NO_NULL_ARGS(1, 1);


IODevice* DeviceTreeGetDeviceWithPath(IODevice* root, const char* path) NO_NULL_POINTERS;


