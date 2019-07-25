

#pragma once

#define PAGE_BITS_4K 4

#define seL4_Fault_NullFault 0
#define seL4_MaxPrio 255
#define seL4_WordBits 4
#define seL4_CapInitThreadCNode 4

typedef long seL4_CPtr;
typedef long seL4_Word;

typedef int cspacepath_t;

typedef struct{} seL4_MessageInfo_t;


static inline seL4_MessageInfo_t seL4_MessageInfo_new(int a, int b, int c , int d)
{
    seL4_MessageInfo_t t;
    return t;
}
