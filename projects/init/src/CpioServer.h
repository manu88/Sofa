#pragma once


#include "Bootstrap.h"
#include "FileServer.h"


FileServerHandler* getCPIOServerHandler(void);

int CPIOServerInit(void);

//int FileServerOpen(InitContext* context , const char*pathname , int flags);
