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

#include <platsupport/local_time_manager.h>
#include <sel4/types.h>


#include <stdint.h>

typedef struct _Thread Thread;

int TimerInit(ps_io_ops_t *ops , seL4_CPtr notifCap);

int TimerAllocAndRegister(time_manager_t *tm , uint64_t period_ns, uint64_t start, uint32_t id, timeout_cb_fn_t callback, uintptr_t token);

int TimerAllocAndRegisterOneShot(time_manager_t *tm , uint64_t rel_ns, uint32_t id,  timeout_cb_fn_t callback, uintptr_t token);


int TimerCancelID( uint32_t id);

int TimerProcess(seL4_Word sender_badge);
time_manager_t *getTM(void);


uint64_t GetCurrentTime(void);


