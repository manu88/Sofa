#include "SysCallsList.h"


long sofa_write(va_list args)
{
        int fd  = va_arg(args, int);
        const char *buf =  va_arg(args, char*);
        size_t count = va_arg(args, size_t);

        // 0 : sysNum
        // 1 : fd
        // 2 : size
        seL4_MessageInfo_t tag = seL4_MessageInfo_new(0, 0, 0, 3 + count);
        seL4_SetMR(0, __SOFA_NR_write);
        seL4_SetMR(1, fd);
        seL4_SetMR(2, count);
        
        for(int i =0;i<count;++i)
        {
                seL4_SetMR(3+i , buf[i]);
        }

        tag = seL4_Call(sysCallEndPoint, tag);
        assert(seL4_GetMR(0) == __SOFA_NR_write);

        return seL4_GetMR(1);
}

