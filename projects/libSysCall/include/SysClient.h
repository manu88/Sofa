#pragma once
#include <sel4/types.h>


int SysClientInit(int argc , char* argv[] );



// IPC Part

int IPCMessageReset(void);

void IPCPushWord(seL4_Word word);
void IPCPushString(const char* str);
