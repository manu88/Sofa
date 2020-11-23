#include <platsupport/io.h>
#include <platsupport/plat/acpi/acpi.h>
#include <AMLDecompiler.h>
#include "DeviceTree.h"
#include "Environ.h"
#include <pci/pci.h>



static int _startDevice(AMLDecompiler* decomp,const ParserContext* context, const ACPIDevice* dev)
{
    char* realName  = AMLNameConstructNormalized(&dev->name);
    printf("Start device '%s'\n", realName);
    free(realName);
    return 0;
}

static int _endDevice(AMLDecompiler* decomp,const ParserContext* context, const ACPIDevice* dev)
{
    char* realName  = AMLNameConstructNormalized(&dev->name);
    printf("End device '%s'\n", realName);
    free(realName);
    return 0;
}

static int _startName(AMLDecompiler* decomp,const ParserContext* context, const char* name)
{
    printf("\tStart Name '%s'\n", name);
    return 0;
}

static int _onValue(AMLDecompiler* decomp,const ParserContext* context, uint64_t value)
{
    printf("\t\tvalue = 0X%lX\n", value);
    return 0;
}

int DeviceTreeInit()
{
    KernelTaskContext* env = getKernelTaskContext();
    int error = 0;
    
    printf("#### PCI SCAN\n");
    libpci_scan(env->ops.io_port_ops);
    printf("#### PCI SCAN\n");
    ps_io_mapper_t io_mapper;
    error =  sel4platsupport_new_io_mapper(env->vspace, env->vka, &io_mapper);
    assert(error == 0);
    
    acpi_t* acpi = acpi_init(io_mapper);
    
    assert(acpi != NULL);

    acpi_header_t* dsdt = acpi_find_region(acpi, ACPI_DSDT);
    assert(dsdt != NULL);

    size_t acpiBufferSize = dsdt->length;

    AMLDecompiler decomp;
    AMLDecompilerInit(&decomp);
    AMLDecompilerUseDefaultCallbacks(&decomp);
    decomp.callbacks.startDevice = _startDevice;
    decomp.callbacks.endDevice = _endDevice;
    decomp.callbacks.startName = _startName;
    decomp.callbacks.onValue = _onValue;
    AMLParserError err = AMLDecompilerStart(&decomp, (const uint8_t*) dsdt, acpiBufferSize);
    
    assert( err == AMLParserError_None);
    
    return 0;
}