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

#include "SysCalls.h"
#include "SysCaps.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sel4utils/vspace.h>
#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#include <assert.h>
#include <string.h>

#include <sel4utils/thread.h>
#include <sel4utils/thread_config.h>

// from kernel_task's Config.h
//#define CONFIG_LIB_OSAPI_ROOT_UNTYPED_MEM_SIZE 67108864
//#define SEL4OSAPI_ROOT_TASK_UNTYPED_MEM_SIZE        CONFIG_LIB_OSAPI_ROOT_UNTYPED_MEM_SIZE

#define CONFIG_LIB_OSAPI_USER_UNTYPED_MEM_SIZE 64*1024*1024
#define SEL4OSAPI_USER_PROCESS_UNTYPED_MEM_SIZE CONFIG_LIB_OSAPI_USER_UNTYPED_MEM_SIZE


/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 4000)
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 20)
static uint8_t _bootstrap_mem_pool[ALLOCATOR_STATIC_POOL_SIZE/*SEL4OSAPI_BOOTSTRAP_MEM_POOL_SIZE*/];

static seL4_CPtr endpoint = 0;
static ThreadEnvir* env = NULL;


static sel4utils_thread_t testThread;

/* --------- System struct */
static sel4osapi_process_env_t* procEnv = NULL;

static allocman_t *allocator = NULL;

static vka_t vka;

static reservation_t vmem_reservation;

void *vmem_addr = NULL;

static vspace_t vspace;

static sel4utils_alloc_data_t vspace_alloc_data;


/* --------- END OF System struct */
vka_t* getVka()
{
    return &vka;
}
vspace_t* getVspace()
{
    return &vspace;
}


void threadStart(void *arg0, void *arg1, void *ipc_buf)
{
    while(1)
    {
        
    }
    
}

static int BoostrapProcess(void)
{
    
    int error = 0;
    
    assert(procEnv);
    
    print("procEnv->free_slots.start %lu\n",procEnv->free_slots.start);
    print("procEnv->free_slots.end %lu\n",procEnv->free_slots.end);
    
    allocator = bootstrap_use_current_1level(procEnv->root_cnode,
                                             procEnv->cspace_size_bits,
                                             procEnv->free_slots.start,
                                             procEnv->free_slots.end,
                                             ALLOCATOR_STATIC_POOL_SIZE,
                                             //SEL4OSAPI_ROOT_TASK_UNTYPED_MEM_SIZE,
                                             _bootstrap_mem_pool);
    
    assert(allocator);
    
    allocman_make_vka(&vka, allocator);
    
    
    print("allocator ok\n");
    int slot, size_bits_index;
    
    cspacepath_t path;
    
    for (slot = procEnv->untypeds.start, size_bits_index = 0;
         slot <= procEnv->untypeds.end;
         slot++, size_bits_index++)
    {
        //print("vka_cspace_make_path at slot %i\n" ,size_bits_index );
        
        vka_cspace_make_path(&vka, slot, &path);
        /* allocman doesn't require the paddr unless we need to ask for phys addresses,
         * which we don't. */
        uintptr_t fake_paddr = 0;
        size_t size_bits = procEnv->untyped_size_bits_list[size_bits_index];
        int error = allocman_utspace_add_uts(allocator, 1, &path, &size_bits, &fake_paddr, ALLOCMAN_UT_KERNEL);
        
        if( error)
        {
            print("allocman_utspace_add_uts error at slot %i\n" ,size_bits_index );
        }
        assert(!error);
    }
    
    print("loop ok\n");
    
    /* create a vspace */
    
#if 1 /*sel4test's version */
    
    void *existing_frames[procEnv->stack_pages + 2];
    existing_frames[0] = (void *) procEnv;
    existing_frames[1] = seL4_GetIPCBuffer();
    assert(procEnv->stack_pages > 0);
    
    for (int i = 0; i < procEnv->stack_pages; i++)
    {
        existing_frames[i + 2] = procEnv->stack + (i * PAGE_SIZE_4K);
    }
    
    error = sel4utils_bootstrap_vspace(&vspace, &vspace_alloc_data, procEnv->page_directory, &vka,
                                       NULL, NULL, existing_frames);
    
    
#else
    void *existing_frames[] = { procEnv,seL4_GetIPCBuffer()};// , NULL};
    error = sel4utils_bootstrap_vspace(&vspace, &vspace_alloc_data, procEnv->page_directory, &vka,
                                       NULL, NULL, existing_frames);
    
#endif /*sel4osapi's version */
    
    
    assert(error == 0);
    print("vspace ok\n");
    
    /*
    error = sel4utils_reserve_range_no_alloc(&vspace , reservation_to_res(vmem_reservation) , SEL4OSAPI_USER_PROCESS_UNTYPED_MEM_SIZE / 4,
                                             seL4_AllRights,
                                             1, &vmem_addr);
    */
    
    
    vmem_reservation = vspace_reserve_range(&vspace,
                                            ALLOCATOR_VIRTUAL_POOL_SIZE/*SEL4OSAPI_USER_PROCESS_UNTYPED_MEM_SIZE / 4*/,
                                            seL4_AllRights,
                                            1, &vmem_addr);
    
    assert(vmem_reservation.res);
    print("vspace_reserve_range ok\n");


    
/*
    print("Try to alloc endpoint\n");
    vka_object_t rootTaskEP;
    error = vka_alloc_endpoint( &vka, &rootTaskEP);
    
    if( error != 0)
    {
        print("vka_alloc_endpoint for RootEndPoint failed %i\n" , error);
     
        return error;
    }
*/
    
    

/* TEST THREAD */
    sel4utils_thread_config_t config = {0};
    
    config = thread_config_fault_endpoint(config, procEnv->fault_endpoint);
    
    seL4_Word data = seL4_CNode_CapData_new(0, seL4_WordBits - procEnv->cspace_size_bits).words[0];
 
    print("Thread Test 1\n");
    config = thread_config_cspace(config, procEnv->root_cnode, data);
    print("Thread Test 2\n");
    config = thread_config_auth(config, procEnv->tcb);
    
    
    int priority = 20;
    print("Thread Test 3\n");
    config = thread_config_mcp(config, (uint8_t)priority);
    print("Thread Test 4\n");
    config = thread_config_priority(config, (uint8_t)priority);
    print("Thread Test 5\n");
    
    config = thread_config_create_reply(config);
    
    error = sel4utils_configure_thread_config(&vka, &vspace, &vspace, config, &testThread);
    if( error != 0)
        print("Error %i\n", error);
    
    assert(error == 0);
    print("Thread Test 6\n");
    error = sel4utils_start_thread(&testThread, threadStart, &testThread, NULL, 1);
    assert(error == 0);
    
    print("Thread started\n");
/* END TEST THREAD */
    return error;//allocator != NULL;
}
int InitClient(const char* EPString )
{
	endpoint = (seL4_CPtr) atoi(EPString );

    seL4_Word badge;
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    //seL4_Recv(endpoint, &badge);
    seL4_SetMR(0 , SysCall_BootStrap);
    
    
    seL4_Call(endpoint, info);
    
    assert(seL4_MessageInfo_get_label(info) == seL4_Fault_NullFault);
    assert(seL4_MessageInfo_get_length(info) == 2);
    assert(seL4_GetMR(0) == SysCall_BootStrap);
    
    //env = (ThreadEnvir*)seL4_GetMR(1);
    procEnv = (sel4osapi_process_env_t*) seL4_GetMR(1);
    assert(procEnv);
    env = &procEnv->mainThreadEnv;
    
    assert(env);
    
    BoostrapProcess();
    
	return 0;
}

void StopClient(int retCode)
{
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Exit);
    seL4_SetMR(1 , retCode);
    
    seL4_Send(endpoint, info);
}


void doExit(int retCode)
{
    StopClient(retCode);
    assert(0); // never returns
    while(1){}
}

void print(const char *format, ...)
{
	assert(env);
	
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
	seL4_SetMR(0 , SysCall_Write);
	// 2nd reg is for return value;

	va_list vl;
    va_start(vl, format);

    vsnprintf( env->buf, IPC_BUF_SIZE, format, vl);

    va_end( vl);

	seL4_Call(endpoint, info);

	assert(seL4_GetMR(0) == SysCall_Write);
}


int spawn(const char* name,int argc , char* argv[])
{
	assert(env);
        
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Spawn);
    // 2nd reg is for return value;

	snprintf( env->buf, IPC_BUF_SIZE, "%s" , name);

    seL4_Call(endpoint, info);

    assert(seL4_GetMR(0) == SysCall_Spawn);
	return (int) seL4_GetMR(1);
}

int kill(int pid)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Kill);
    seL4_SetMR(1 , pid);
    // 2nd reg is for return value;
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_Kill);
    return (int) seL4_GetMR(1);
}

long wait(int *wstatus ,SofaSignal* sign)
{
    if( wstatus == NULL)
        return -1;
    if( sign == NULL)
        return -1;
    
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 4);
    seL4_SetMR(0 , SysCall_Wait);
    // 2nd reg is for returned pid;
    // 3rd reg is for returned status;
    // 4th reg is for signal, -1 if invalid
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_Wait);
    *wstatus = (int) seL4_GetMR(2);
    
    int sigN = (int) seL4_GetMR(3);
    if( sigN > -1 )
    {
        *sign = (SofaSignal) sigN;
    }
    return (long) seL4_GetMR(1);
}

long unsigned int getTime()
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_GetTime);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_GetTime);
    
    return seL4_GetMR(1);
}

static int _Sleep( unsigned long l , SleepUnit unit)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_Sleep);
    seL4_SetMR(1 , l);
    seL4_SetMR(2 , unit);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_Sleep);
    
    return (int) seL4_GetMR(1);
}

int sleepS( unsigned long sec)
{
    return _Sleep(sec , SleepUnit_S);
}

int sleep( unsigned long ns)
{
    return _Sleep(ns , SleepUnit_NS);
}

int sleepMS( unsigned long ms)
{
    return _Sleep(ms , SleepUnit_MS);
}

long read(char*buf, unsigned long count)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    seL4_SetMR(0 , SysCall_Read);
    seL4_SetMR(1 , count);
    
    seL4_Call(endpoint, info);

    assert(seL4_GetMR(0) == SysCall_Read);
    memcpy(buf ,env->buf, seL4_GetMR(1) );
    return (long) seL4_GetMR(1);
}

static void doDebugSysCall(SysCall_Debug_ID msgID)
{
    assert(env);
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    
    seL4_SetMR(0 , SysCall_Debug);
    seL4_SetMR(1 , msgID);
    
    seL4_Send(endpoint, info);
}

void ps()
{
    doDebugSysCall(SysCall_Debug_PS);
}

void sched()
{
    doDebugSysCall(SysCall_Debug_Sched);
}

void listServers()
{
    doDebugSysCall(SysCall_Debug_ListServers);
}

static long doIDsSysCall(SysCallGetIDs_OP op)
{
    assert(env);
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    
    seL4_SetMR(0 , SysCall_GetIDs);
    seL4_SetMR(1 , op);
    
    seL4_Call(endpoint, info);
    
    return seL4_GetMR(1);
}

int getPID()
{
    return (int) doIDsSysCall(SysCallGetIDs_GetPID);
}

int getParentPID()
{
    return (int) doIDsSysCall(SysCallGetIDs_GetPPID);
}

int getPriority(int pid , int *retVal)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_GetPriority);
    seL4_SetMR(1 , pid); // will contain error
    // 2 : returnedprio
    
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_GetPriority);
    
    *retVal = seL4_GetMR(2);
    return seL4_GetMR(1);
}

int setPriority(int pid , int prio)
{
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_SetPriority);
    seL4_SetMR(1 , pid); // will contain error
    seL4_SetMR(2 , prio);
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_SetPriority);
    
    return seL4_GetMR(1);
}

ServerEnvir* RegisterServerWithName(const char*name, int flags)
{
    if( !name )
        return NULL;
    
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 4);
    seL4_SetMR(0 , SysCall_RegisterServer);
    seL4_SetMR(1 , flags);
    // 2nd reg is for return value;
    // 3rd will get the ServerEnv pointer
    
    snprintf( env->buf, IPC_BUF_SIZE, "%s" , name);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_RegisterServer);
    
    if( seL4_GetMR(1) == 0)
    {
        
        seL4_CPtr serverEP = seL4_GetMR(3);
        ServerEnvir* ret = (ServerEnvir*) seL4_GetMR(2);
        
        /*
        seL4_Word sender_badge = 0;
        print("Server Wait for a message !\n");
        seL4_MessageInfo_t message = seL4_Recv(serverEP, &sender_badge);
        print("Got a message !\n");
        */
        return ret;
    }
    
    return NULL;
}

ClientEnvir* ConnectToServer( const char*name)
{
    if( !name )
        return NULL;
    
    assert(env);
    
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    seL4_SetMR(0 , SysCall_RegisterClient);
    // 2nd reg is for return value;
    // 3rd will get the ServerEnv pointer
    
    snprintf( env->buf, IPC_BUF_SIZE, "%s" , name);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_RegisterClient);
    
    if( seL4_GetMR(1) == 0)
    {
        return (ClientEnvir*) seL4_GetMR(2);
    }
    
    return NULL;
}

int ServerRecv(ServerEnvir* server)
{
    seL4_Word sender_badge = 0;
    seL4_MessageInfo_t message = seL4_Recv(server->endpoint, &sender_badge);
    
    return (int)sender_badge;
}




/*
long sel4_vsyscall(long sysnum, ...)
{
    assert(0);
    return 0;
}
*/

/* Put a pointer to sel4_vsyscall in a special section so anyone loading us
 * knows how to configure our syscall table */
//uintptr_t VISIBLE SECTION("__vsyscall") __vsyscall_ptr = (uintptr_t) sel4_vsyscall;



/* **** **** **** **** **** **** **** */
/* Sys caps*/


static seL4_Word doCapSysCall(CapOperation capOP , seL4_Word data1)
{
    assert(env);
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 3);
    
    seL4_SetMR(0 , SysCall_CapOp);
    seL4_SetMR(1 , capOP);
    seL4_SetMR(2 , data1);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_CapOp);
    
    return seL4_GetMR(2);
}

void CapDrop( SofaCapabilities cap)
{
    doCapSysCall(CapOperation_Drop , cap);
}

int  CapAcquire( SofaCapabilities cap)
{
    doCapSysCall(CapOperation_Acquire , cap);
}

void* RequestResource( SofaResource res)
{
    assert(env);
    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 2);
    
    seL4_SetMR(0 , SysCall_ResourceReq);
    
    seL4_Call(endpoint, info);
    
    assert(seL4_GetMR(0) == SysCall_ResourceReq);
    
    return (void*)seL4_GetMR(1);
}
