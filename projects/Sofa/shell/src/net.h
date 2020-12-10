#pragma once
#include <stddef.h>
#include <sys/types.h>


int NetInit(void);


int NetBind(int familly, int protoc, int port);