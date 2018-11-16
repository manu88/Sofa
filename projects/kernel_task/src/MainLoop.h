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

#include "Bootstrap.h"
#include "ProcessDef.h"

#define IRQ_EP_BADGE       BIT(seL4_BadgeBits - 1)
#define IRQ_BADGE_TIMER    (1 << 0)
#define IRQ_BADGE_NETWORK  (1 << 1)
#define IRQ_BADGE_KEYBOARD (1 << 2)

//void handle_cdev_event( void* dev); 

int UpdateTimeout(InitContext* context,uint64_t timeNS);

// never returns
void processLoop(InitContext* context, seL4_CPtr epPtr);

