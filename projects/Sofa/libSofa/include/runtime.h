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

#include "test.h"
#include "test_init_data.h"



typedef struct
{
    seL4_CPtr ep;
    uint8_t* buffer;
} TLSContext;


void TLSSet( TLSContext* ctx);
TLSContext* TLSGet(void);

void init_allocator(env_t env, test_init_data_t *init_data);
void init_simple(env_t env, test_init_data_t *init_data);

int RuntimeInit(int argc, char *argv[]);
int RuntimeInit2(int argc, char *argv[]);

seL4_CPtr getProcessEndpoint(void);
struct env* getProcessEnv(void);

seL4_CPtr getNewThreadEndpoint(uint8_t** ipcBufferAddr);

void sendThreadExit(seL4_CPtr ep);