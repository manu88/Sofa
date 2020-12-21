#pragma once
#include <stddef.h>
#include <sys/types.h>

int VFSClientInit(void);

int VFSClientOpen(const char* path, int mode);
int VFSClientClose(int handle);

ssize_t VFSClientRead(int handle, char* data, size_t size);
ssize_t VFSClientWrite(int handle, const char* data, size_t size);

int VFSClientSeek(int handle, size_t pos);

void VFSClientDebug(void);

int Printf(const char *format, ...) __attribute__((format(printf,1,2)));