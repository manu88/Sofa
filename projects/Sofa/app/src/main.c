#include "test_init_data.h"
#include "test.h"

static seL4_CPtr endpoint;



int main(int argc, char *argv[])
{
    test_init_data_t *init_data;
    struct env env;

    /* parse args */
    assert(argc == 2);
    endpoint = (seL4_CPtr) atoi(argv[0]);

    /* read in init data */
    init_data = (void *) atol(argv[1]);


    seL4_MessageInfo_t info = seL4_MessageInfo_new(seL4_Fault_NullFault, 0, 0, 1);
    seL4_SetMR(0, 42);
    seL4_Send(endpoint, info);

    return 0;
}

