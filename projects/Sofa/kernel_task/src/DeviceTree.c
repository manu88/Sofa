#include <platsupport/io.h>
#include <platsupport/plat/acpi/acpi.h>
#include <AMLDecompiler.h>
#include "DeviceTree.h"
#include "Environ.h"
#include "Net.h"
#include <pci/pci.h>
#include <ctype.h>



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

static int _onMethod(AMLDecompiler* decomp,const ParserContext* context, const ACPIMethod* method)
{
    return 0;
    if(strcmp(method->name, "_PRT") == 0)
    {
        uint8_t* b = method->bodyDef;
        
        for (int i=0;i<16;i++)
        {
            if(i%8 == 0)
            {
                printf("\n");
            }
            uint8_t c = b[-16 + i];

            printf(" 0X%X (%c) ", c, isprint(c)? method->bodyDef[i]: ' ');
        }
        printf("###\n");

    }
    printf("Got Method '%s' size %zi\n", method->name, method->bodySize);
    for (int i=0;i<method->bodySize;i++)
    {
        if(i%8 == 0)
        {
            printf("\n");
        }

        printf(" 0X%X (%c) ", method->bodyDef[i], isprint(method->bodyDef[i])? method->bodyDef[i]: ' ');
    }
    printf("\n");

    if(strcmp(method->name, "_PRT") == 0)
    {
        assert(0);
    }
    return 0;
}


static int _onLargeItem(AMLDecompiler* decomp,const ParserContext* context, LargeResourceItemsType itemType, const uint8_t* buffer , size_t bufferSize)
{
    if(itemType != LargeResourceItemsType_ExtendedIRQDescriptor)
        return 0;
    
    printf("On IRQ desc\n");

    const ExtendedInterruptDescriptor* desc = (const ExtendedInterruptDescriptor*) buffer;
    for (int i=0;i<desc->interruptTableLen;i++)
    {
        printf("IRQ %i\n", desc->interruptNumber[i]); 
    }
    return 0;
}
static int _onSmallItem(AMLDecompiler* decomp,const ParserContext* context, SmallResourceItemsType itemType, const uint8_t* buffer , size_t bufferSize)
{
    printf("On small Item %s\n", itemType == SmallResourceItemsType_IRQFormatDescriptor? "IRQ desc":"");

    if(itemType == SmallResourceItemsType_IRQFormatDescriptor)
    {
        const IRQDescriptor* desc = (const IRQDescriptor*) buffer;
        printf("maskBits 0X%X\n", desc->maskBits);
    }
    return 0;
}


static int _startPackage(AMLDecompiler* decomp,const ParserContext* context, const ACPIPackage* package)
{
    printf("Start package\n");
    return 0;
}
static int _onPackageElement(AMLDecompiler* decomp,const ParserContext* context, const ACPIPackageElement* element)
{
    printf("\ton package elt\n");
    return 0;
}
static int _endPackage(AMLDecompiler* decomp,const ParserContext* context, const ACPIPackage* package)
{
    printf("end package\n");
    return 0;
}

static int _startResourceTemplate(AMLDecompiler* decomp, const ParserContext* context , size_t numItems )
{
    printf("_startResourceTemplate\n");
    return 0;
}
static int _endResourceTemplate(AMLDecompiler* decomp, const ParserContext* context , size_t numItemsParsed, AMLParserError err)
{
    printf("_endResourceTemplate\n");
    return 0;
}


static void ACPITest()
{
    KernelTaskContext* env = getKernelTaskContext();
    // don't bother APCI scan for now
    ps_io_mapper_t io_mapper;
    int error =  sel4platsupport_new_io_mapper(env->vspace, env->vka, &io_mapper);
    assert(error == 0);
    
    acpi_t* acpi = acpi_init(io_mapper);
    
    assert(acpi != NULL);

    acpi_header_t* dsdt = acpi_find_region(acpi, ACPI_DSDT);
    assert(dsdt != NULL);

    size_t acpiBufferSize = dsdt->length;


    AMLDecompiler decomp;
    AMLDecompilerInit(&decomp);
    AMLDecompilerUseDefaultCallbacks(&decomp);
    decomp.callbacks.onSmallItem = _onSmallItem;
    decomp.callbacks.onLargeItem = _onLargeItem;
    decomp.callbacks.startDevice = _startDevice;
    decomp.callbacks.endDevice = _endDevice;
    decomp.callbacks.startName = _startName;
    decomp.callbacks.onValue = _onValue;
    decomp.callbacks.onMethod = _onMethod;
    decomp.callbacks.startPackage = _startPackage;
    decomp.callbacks.onPackageElement = _onPackageElement;
    decomp.callbacks.endPackage = _endPackage;
    decomp.callbacks.startResourceTemplate = _startResourceTemplate;
    decomp.callbacks.endResourceTemplate = _endResourceTemplate;
    AMLParserError err = AMLDecompilerStart(&decomp, (const uint8_t*) dsdt, acpiBufferSize);
    
    assert( err == AMLParserError_None);

}

int DeviceTreeInit()
{
    ACPITest();

    KernelTaskContext* env = getKernelTaskContext();
    int error = 0;
    
    printf("#### PCI SCAN\n");
    libpci_scan(env->ops.io_port_ops);
    printf("#### PCI SCAN\n");

    printf("Got %u pci devices\n", libpci_num_devices);

    //vid 0x1af4 did 0x1000
    libpci_device_t *virtioDev = libpci_find_device(0x1af4, 0x1000);

    if(virtioDev)
    {
        printf("Got Virtio device '%s' from '%s' subsystem %i\n", virtioDev->vendor_name, virtioDev->device_name, virtioDev->subsystem_id);
        if(virtioDev->subsystem_id == 1) // network card
        {
            libpci_device_iocfg_t iocfg;
            libpci_read_ioconfig(&iocfg, virtioDev->bus, virtioDev->dev, virtioDev->fun);
            libpci_device_iocfg_debug_print(&iocfg, false);
            uint32_t iobase0 =  libpci_device_iocfg_get_baseaddr32(&iocfg, 0);
            printf("IOBASE0 is %X\n", iobase0);
            NetInit(iobase0);
        }
        else
        {
            printf("Virtio device subsystem is %i\n", virtioDev->subsystem_id);
        }
    }

    return 0;
    
}