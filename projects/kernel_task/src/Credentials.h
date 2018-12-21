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


#include <stdint.h>
#include "Sofa.h"

#define _KERNEL_CAPABILITY_U64S 1

typedef struct
{
	uint64_t cap[_KERNEL_CAPABILITY_U64S];
} Capacity;

#define ROOT_IDENTITY 0


typedef struct
{
    uint64_t uid;
} Identity;

int IdentityHasAuthority( const Identity* refIdentity, const Identity* identityToCheck) SOFA_UNIT_TESTABLE  NO_NULL_POINTERS;
