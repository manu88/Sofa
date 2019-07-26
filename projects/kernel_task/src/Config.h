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


#define SOFA_DEFAULT_PRIORITY 100

/*
 * Amount of untyped memory to reserve for a user process
 *
 * Default: 64MB
 *
 */
#define CONFIG_LIB_OSAPI_USER_UNTYPED_MEM_SIZE 67108864
#define SEL4OSAPI_USER_PROCESS_UNTYPED_MEM_SIZE CONFIG_LIB_OSAPI_USER_UNTYPED_MEM_SIZE
