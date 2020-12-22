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
#include <lwip/netif.h>
#include "KThread.h"



typedef void (*netif_handle_irq_fn)(void *state, int irq_num);

typedef struct
{
    int irq_num;
    netif_init_fn init_fn;
    void *driver;
    netif_handle_irq_fn handle_irq_fn;

}NetworkDriver;



void NetInit(uint32_t iobase0);

