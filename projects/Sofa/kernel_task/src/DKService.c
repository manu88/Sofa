#include "DKService.h"
#include "NameServer.h"
#include "Environ.h"

static Service _dkService;
static char _dkName[] = "deviceKit";

int DKServiceInit()
{
        int error = 0;
    ServiceInit(&_dkService, getKernelTaskProcess() );
    _dkService.name = _dkName;

    ServiceCreateKernelTask(&_dkService);
    NameServerRegister(&_dkService);
    return 0;

}