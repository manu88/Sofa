#pragma once

#include <utils/compile_time.h>
#include <utils/page.h>
#include <sel4/sel4.h>
#include <vka/vka.h>
#include <vspace/vspace.h>

#define SOFA_PROCESS_CSPACE_SIZE_BITS 17

typedef struct
{
    /* An initialised vka. Setup by the process itself */
    vka_t vka;
    /* virtual memory management interface */
    vspace_t vspace;
    /* page directory of the process */
    seL4_CPtr page_directory;
    /* root cnode of the process */
    seL4_CPtr root_cnode;

    /* size of the  process cspace */
    seL4_Word cspace_size_bits;

    /* tcb of the process */
    seL4_CPtr tcb;
    
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