#pragma once

#include "test.h"


typedef struct _UntypedRange
{
    int start;
    int size;

}UntypedRange;


/* list of untypeds to give out to test processes */
vka_object_t* getUntypeds(void);

/* list of sizes (in bits) corresponding to untyped */
uint8_t* GetUntypedSizeBitsList(void);


int UntypedsGetFreeRange(UntypedRange* range);

void UnypedsGiveBack(const UntypedRange* range);

void printUntypedRange(void);



void *kmalloc(size_t size);
void kfree(void *ptr);
