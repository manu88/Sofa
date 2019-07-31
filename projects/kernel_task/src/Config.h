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
#define CONFIG_LIB_OSAPI_USER_UNTYPED_MEM_SIZE 64*1024*1024
#define SEL4OSAPI_USER_PROCESS_UNTYPED_MEM_SIZE CONFIG_LIB_OSAPI_USER_UNTYPED_MEM_SIZE


/* Amount of untyped memory to reserve for the root_task
*
* Default: 64MB

*/
#define CONFIG_LIB_OSAPI_ROOT_UNTYPED_MEM_SIZE 64*1024*1024
#define SEL4OSAPI_ROOT_TASK_UNTYPED_MEM_SIZE        CONFIG_LIB_OSAPI_ROOT_UNTYPED_MEM_SIZE

/*
 * Minimum amount of memory required to bootstrap
 * the root memory allocator.
 *
 * Default: 40K
 */
#define CONFIG_LIB_OSAPI_BOOTSTRAP_MEM_POOL_SIZE 40*1024
#define SEL4OSAPI_BOOTSTRAP_MEM_POOL_SIZE CONFIG_LIB_OSAPI_BOOTSTRAP_MEM_POOL_SIZE



#define CONFIG_LIB_OSAPI_VKA_VMEM_SIZE 64*1024*1024
#define SEL4OSAPI_VKA_VMEM_SIZE CONFIG_LIB_OSAPI_VKA_VMEM_SIZE


/*
 * Minimum amount of virtual memory memory required
 * by the root memory allocator.
 *
 * Default: 64MB
 *
 */
#define CONFIG_LIB_OSAPI_SYSTEM_VMEM_SIZE 64*1024*1024
#define SEL4OSAPI_SYSTEM_VMEM_SIZE                  CONFIG_LIB_OSAPI_SYSTEM_VMEM_SIZE
