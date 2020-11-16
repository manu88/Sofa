#pragma once
#include "Environ.h"

//#define SERIAL_BADGE 2

#define SERIAL_CIRCULAR_BUFFER_SIZE 512
int SerialInit(void);

void handleSerialInput(KernelTaskContext* env);


typedef void (*OnBytesAvailable)(size_t size, char until, void* ptr);

size_t SerialGetAvailableChar(void);
size_t SerialCopyAvailableChar(char* dest, size_t maxSize);
int SerialRegisterWaiter(OnBytesAvailable callback, size_t forSize, char until, void* ptr);