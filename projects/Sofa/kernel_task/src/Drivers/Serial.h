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
#include "Environ.h"

//#define SERIAL_BADGE 2

#define SERIAL_CIRCULAR_BUFFER_SIZE 512
int SerialInit(void);

void handleSerialInput(KernelTaskContext* env);


typedef void (*OnBytesAvailable)(size_t size, char until, void* ptr, void* ptr2);
typedef void (*OnControlChar)(char ctl, void* ptr);

size_t SerialGetAvailableChar(void);
size_t SerialCopyAvailableChar(char* dest, size_t maxSize);

int SerialRegisterWaiter(OnBytesAvailable callback, size_t forSize, char until, void* ptr, void* ptr2);
int SerialRegisterController(OnControlChar callback, void* ptr);