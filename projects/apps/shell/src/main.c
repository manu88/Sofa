#include <SysClient.h>

int main( int argc , char* argv[])
{
    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }

    return 0;
}
