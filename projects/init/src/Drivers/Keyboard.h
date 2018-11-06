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
#include "../IODevice.h"
#include "../Sofa.h"
#include "../Bootstrap.h"
typedef struct
{

    IOBaseDevice super;

    InitContext* context;
    /* platsupport char device */
    ps_chardevice_t dev;
    /* IRQHandler cap (with cspace path) */
    cspacepath_t handler;
    /* endpoint cap (with cspace path) device is waiting for IRQ */
    cspacepath_t ep;


} KeyboardDevice;




int KeyboardDeviceInit(InitContext* context,const cspacepath_t* notificationSrc, KeyboardDevice* keyboard) NO_NULL_POINTERS;
