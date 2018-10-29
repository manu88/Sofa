#pragma once


#include "Bootstrap.h"




int FileServerInit(void);

int FileServerOpen(InitContext* context , const char*pathname , int flags);
