#pragma once
#include <stddef.h>
#include <sys/types.h>

int VFSClientInit(void);

int VFSOpen(const char* path, int mode);
int VFSClose(int handle);

ssize_t VFSRead(int handle, char* data, size_t size);
ssize_t VFSWrite(int handle, const char* data, size_t size);

int VFSSeek(int handle, size_t pos);

void VFSDebug(void);

int Printf(const char *format, ...) __attribute__((format(printf,1,2)));