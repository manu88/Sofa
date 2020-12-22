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





/* list of untypeds to give out to test processes */
vka_object_t* getUntypeds(void);

/* list of sizes (in bits) corresponding to untyped */
uint8_t* GetUntypedSizeBitsList(void);

void *kmalloc(size_t size);
void kfree(void *ptr);
