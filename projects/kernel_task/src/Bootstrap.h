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

#include <simple/simple.h>
#include <allocman/vka.h>
#include <platsupport/chardev.h>





typedef struct
{
    vka_object_t rootTaskEP;
    ps_chardevice_t comDev;
    
    //System* system;
} KernelTaskContext;

int bootstrapSystem(void);
int bootstrapIO(void);


simple_t* getSimple(void);
vka_t* getVka(void);
vspace_t* getVspace(void);
struct ps_io_ops* getIO_OPS(void);

typedef struct sel4osapi_system sel4osapi_system_t; // forward

sel4osapi_system_t* getSystem(void);

KernelTaskContext* getKernelTaskContext(void);


