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
#include <sel4/sel4.h>
#include <allocman/vka.h> // for PAGE_SIZE_4K definitions. Maybe an other header should be more suitable but I can't find where PAGE_SIZE_4K is defined...
#include <stdint.h>


typedef enum
{
    SofaSignal_None = 0,  // no signal
    SofaSignal_Kill = 1,  // eq. to SIGKILL
    SofaSignal_VMFault,   // eq. to SIGSEGV
    
} SofaSignal;

/* ---- Should be private ---- */
typedef enum
{
    SysCall_BootStrap = 1, // 1st msg sent to a process by kernel_task so it can retreive its environment.
    SysCall_GetIDs,
	SysCall_Write,
    SysCall_Sleep,
	SysCall_Read,
	SysCall_Debug,
	SysCall_Spawn,
    SysCall_Kill,
    SysCall_Wait,
    SysCall_Exit,
    SysCall_GetTime,
    
    SysCall_SetPriority,
    SysCall_GetPriority,
    
    SysCall_CapOp,
    SysCall_ResourceReq,
    
    SysCall_RegisterServer,
    SysCall_RegisterClient,
    
    
    SysCall_Last, // MUST REMAINS LAST !
} SysCallID;

/* ---- Should be private ---- */
typedef enum
{
    SysCall_Debug_PS =1,
    SysCall_Debug_Sched,
    SysCall_Debug_ListServers,
}SysCall_Debug_ID;


typedef enum
{
    SysCallGetIDs_GetPID,
    SysCallGetIDs_GetPPID,
} SysCallGetIDs_OP;

/* ---- Should be private ---- */
typedef enum
{
    SleepUnit_NS,
    SleepUnit_MS,
    SleepUnit_S,
    
} SleepUnit;

/* ---- Should be private ---- */
typedef enum
{
    CapOperation_Drop,
    CapOperation_Acquire,
    
} CapOperation;

#ifdef TEST_ONLY
// this value is not relevant for Tests.
#define CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS 10
#endif


#define IPC_BUF_SIZE 256

typedef struct
{
    char buf[IPC_BUF_SIZE];
    unsigned long bufSize;
    
} ThreadEnvir;

/*
 * Data structure representing a process' environment
 * information. This information includes mostly useful
 * capabilities to resources that are used by
 * sel4osapi/applications.
 *
 * The capabilities are always relative to the root
 * cnode of the process.
 */
typedef struct sel4osapi_process_env
{
    /*
     * Page directory used by the process' VSpace
     */
    seL4_CPtr page_directory;
    /*
     * Root cnode of the process' CSpace.
     */
    seL4_CPtr root_cnode;
    /*
     * Size, in bits, of the process' CSpace
     */
    seL4_Word cspace_size_bits;
    /*
     * TCB of the process' initial thread.
     */
    seL4_CPtr tcb;
    /*
     * Range of free capabilities in the
     * process' CSpace.
     */
    seL4_SlotRegion free_slots;
    /*
     * Range of capabilities to untyped
     * objects assigned to the process.
     */
    seL4_SlotRegion untypeds;
    /*
     * Array of indices of the untyped objects
     * that the root task assigned to the process.
     */
    uint8_t untyped_indices[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];
    /*
     * Size, in bits, of each untyped object
     * assigned to the process.
     */
    uint8_t untyped_size_bits_list[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];
    /*
     * Endpoint used as fault endpoint by the
     * process' initial thread.
     */
    seL4_CPtr fault_endpoint;
    
    ThreadEnvir mainThreadEnv;
    
    /* the number of pages in the stack */
    //int stack_pages;
    
    /* address of the stack */
    //void *stack;
    
    /*
     * Endpoint to the sysclock instance.
     */
    //seL4_CPtr sysclock_server_ep;
    /*
     * Async Endpoint used to block the process'
     * threads in idling mode.
     */
    //seL4_CPtr idling_aep;
    
    /*
     * Threads created by this process
     */
    //simple_pool_t *threads;
    
    /*
     * Priority of the process.
     */
    //uint8_t priority;
    
    //sel4osapi_ipcclient_t ipcclient;
    
#ifdef CONFIG_LIB_OSAPI_NET
    //sel4osapi_udp_interface_t udp_iface;
#endif
    
#ifdef CONFIG_LIB_OSAPI_SERIAL
    //sel4osapi_serialclient_t serial;
#endif
    
} sel4osapi_process_env_t;









typedef struct
{
    char buf[IPC_BUF_SIZE];
    unsigned long bufSize;
    
    seL4_CPtr endpoint;
} ServerEnvir;

typedef struct
{
    char buf[IPC_BUF_SIZE];
    unsigned long bufSize;
    
    seL4_CPtr endpoint;
} ClientEnvir;

#ifdef KERNEL_TASK
// an instance of sel4osapi_process_env_t MUST fit in a single page
STATIC_ASSERT(sizeof(sel4osapi_process_env_t) < PAGE_SIZE_4K,sel4osapi_process_env_t);

// dito for other structs
STATIC_ASSERT(sizeof(ThreadEnvir) < PAGE_SIZE_4K,ThreadEnvir);
STATIC_ASSERT(sizeof(ServerEnvir) < PAGE_SIZE_4K,ServerEnvir);
STATIC_ASSERT(sizeof(ClientEnvir) < PAGE_SIZE_4K,ClientEnvir);
#endif

#ifndef KERNEL_TASK

// not public
int InitClient(const char* EPString );
void StopClient(int retCode);

void doExit(int retCode);

void print(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

int spawn(const char* name, int argc , char* argv[]);

long read(char*buf, unsigned long count);

int sleep( unsigned long ns);
int sleepS( unsigned long sec);
int sleepMS( unsigned long ms);
int kill(int pid);

// returns -1 if no child to wait.
long wait(int *wstatus ,SofaSignal* sign);

long unsigned int getTime(void);

void ps(void);
void sched(void);

int getPriority(int pid , int *retVal);
int setPriority(int pid , int prio);


int getPID(void);
int getParentPID(void);

// Server
void listServers(void);
ServerEnvir* RegisterServerWithName(const char*name, int flags);


ClientEnvir* ConnectToServer( const char*name);
int ServerRecv(ServerEnvir* server);



//vka_t* getVka(void);
//vspace_t* getVspace(void);
#endif
