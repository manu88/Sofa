#pragma once

#include "Environ.h"





/* list of untypeds to give out to test processes */
vka_object_t* getUntypeds(void);

/* list of sizes (in bits) corresponding to untyped */
uint8_t* GetUntypedSizeBitsList(void);

void *kmalloc(size_t size);
void kfree(void *ptr);
