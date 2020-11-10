#pragma once

#include <utils/compile_time.h>
#include <utils/page.h>
#include <sel4/sel4.h>
#include <vka/vka.h>
#include <vspace/vspace.h>
#include <simple/simple.h>

#include <sel4utils/thread.h>

#define SOFA_PROCESS_CSPACE_SIZE_BITS 17

#define IPC_BUF_LEN 64
// shared between kernel_task and processes
typedef struct
{

    // the process' PID
    int pid;

    uint8_t ipcBuffer[IPC_BUF_LEN];
    
    /* An initialised vka. Setup by the process itself */
    vka_t vka;
    /* virtual memory management interface */
    vspace_t vspace;
    /* abstract interface over application init */
    simple_t simple;

    
    seL4_CPtr root_cnode; /* root cnode of the process Note: eq. to cspace_root in env_t*/
    seL4_CPtr mainEndpoint;
    seL4_CPtr page_directory; /* page directory of the process */
    seL4_CPtr tcb; /* tcb of the process */

    /* number of available cores */
    seL4_Word cores;
    /* sched control cap */
    seL4_CPtr sched_ctrl;


    /* size of the  process cspace */
    seL4_Word cspace_size_bits;

   
    
    /* range of untyped memory in the cspace */
    seL4_SlotRegion untypeds;
    uint8_t untyped_size_bits_list[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];
    /* range of free slots in the cspace */
    seL4_SlotRegion free_slots;

    /* the number of pages in the stack */
    int stack_pages;

    /* address of the stack */
    void *stack;
} ProcessContext;


compile_time_assert(ProcessContext_fits_in_ipc_buffer, sizeof(ProcessContext) < PAGE_SIZE_4K);


/*
Structure stored in TLS, that will hold all per-thread relevant data
*/
typedef struct
{
    seL4_CPtr endpoint;
} TLSContext;

// FIXME: This is not a public API
void Thread_init_tls(void *thread);