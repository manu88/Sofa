#include "X86Expert.h"
#include "Log.h"
#include "Environ.h"
#include "Blk.h"
#include "DeviceTree.h"
#include <pci/pci.h>
#include <platsupport/plat/acpi/acpi.h>
#include <platsupport/plat/acpi/tables/madt.h>
#include <AMLDecompiler.h>
#include <EISAID.h>


static IONode _rootNode = IONodeNew("\\");

static void ACPITest(void);


int PlatformExpertInit()
{
    KLOG_INFO("PlatformExpertInit for X86_64\n");
    return 0;
}


static void CPUid(uint32_t op, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    __asm__("cpuid"
            : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)   // The output variables. EAX -> a and vice versa.
            : "0" (op));                         // Put the infotype into EAX.
}
int PlatformExpertConstructTree()
{
    ACPITest();

    return 0;
    char procname[13];

    uint32_t aa;

    CPUid(0, &aa, (uint32_t)procname , (uint32_t)(procname + 8) ,(uint32_t)(procname + 4));
    procname[12]=0;
    KLOG_DEBUG("Proc name '%s'\n", procname);

    uint32_t infotype, a, b, c, d;

    for (infotype = 1; infotype < 5; infotype ++)
    {
        CPUid(infotype, &a, &b, &c, &d);
        printf ("InfoType %x\nEAX: %x\nEBX: %x\nECX: %x\nEDX: %x\n", infotype, a, b, c, d);
    }

    KernelTaskContext* env = getKernelTaskContext();
    int error = 0;
    
    printf("#### PCI SCAN\n");
    libpci_scan(env->ops.io_port_ops);
    printf("#### PCI SCAN\n");

    printf("Got %u pci devices\n", libpci_num_devices);

//Storage virtio 
    libpci_device_t *virtioBlkDev = libpci_find_device(0x1af4, 0x1001);
    if(virtioBlkDev)
    {
        printf("Got Virtio Blk device '%s' from '%s' subsystem %i\n", virtioBlkDev->vendor_name, virtioBlkDev->device_name, virtioBlkDev->subsystem_id);

        if(virtioBlkDev->subsystem_id == 2)
        {
            libpci_device_iocfg_t iocfg;
            libpci_read_ioconfig(&iocfg, virtioBlkDev->bus, virtioBlkDev->dev, virtioBlkDev->fun);
            libpci_device_iocfg_debug_print(&iocfg, false);
            uint32_t iobase0 =  libpci_device_iocfg_get_baseaddr32(&iocfg, 0);

            BlkInit(iobase0);
            //virtio_blk_init(iobase0);
        }
    }

//NET virtio: vid 0x1af4 did 0x1000
    libpci_device_t *virtioNetDev = libpci_find_device(0x1af4, 0x1000);

    if(virtioNetDev)
    {
        printf("Got Virtio Net device '%s' from '%s' subsystem %i\n", virtioNetDev->vendor_name, virtioNetDev->device_name, virtioNetDev->subsystem_id);
        if(virtioNetDev->subsystem_id == 1) // network card
        {
            libpci_device_iocfg_t iocfg;
            libpci_read_ioconfig(&iocfg, virtioNetDev->bus, virtioNetDev->dev, virtioNetDev->fun);
            libpci_device_iocfg_debug_print(&iocfg, false);
            uint32_t iobase0 =  libpci_device_iocfg_get_baseaddr32(&iocfg, 0);
            printf("IOBASE0 is %X\n", iobase0);
            NetInit(iobase0);
        }
        else
        {
            printf("Virtio device subsystem is %i\n", virtioNetDev->subsystem_id);
        }
    }

    return 0;
    
}




/***** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

typedef struct
{
    IONode *rootNode;
    
#define MaxScope 64
    char *scopes[64];
    int scopeLevel;


    IONode* currentNode;
} DeviceTreeContext;


IONode* getNode(DeviceTreeContext* ctx, const char* name)
{
    printf("getNode for name '%s'\n", name);
    if(strcmp(name, "\\") == 0)
    {
        if(ctx->currentNode == NULL)
        {
            ctx->currentNode = ctx->rootNode;
        }
        return ctx->rootNode;
    }
    if(name[0] == '\\')
    {
        // start from beginning
        printf("Get node starts from beginning\n");
        name += 1;
    }
    else
    {
        if(strchr(name, '.') != NULL)
        {
            printf("'%s' needs to be walked\n", name);
            char *ptr = strtok(name, ".");

            IONode* nextNode = ctx->currentNode;
            while(ptr != NULL)
            {
                //printf("'%s'\n", ptr);
                IONode* nNext = IONodeGetChildren(nextNode, ptr); 
                if(nNext)
                {
                    nextNode = nNext; 
                }
                else
                {
                    printf("'%s' not found\n", ptr);
                    nNext = malloc(sizeof(IONode));
                    assert(nNext);
                    IONodeInit(nNext, strdup(ptr));
                    IONodeAddChild(nextNode, nNext);
                    nextNode = nNext;
                }
                
                ptr = strtok(NULL, ".");
            }
            printf("End of Walk\n");
            return nextNode;
        }
        else
        {
            IONode* n = IONodeGetChildren(ctx->currentNode, name);
            if(n)
            {
                return n;
            }
        }
    }
      
    printf("Node '%s' not found, create in '%s'\n", name, ctx->currentNode->name);
// Node is NULL, create it

    IONode* newNode = malloc(sizeof(IONode));
    assert(newNode);
    memset(newNode, 0, sizeof(IONode));
    newNode->name = strdup(name);
    printf("Created node '%s'\n", name);

    IONodeAddChild(ctx->currentNode, newNode);

    //ctx->currentNode = newNode;

    return newNode;
}

static void debugScope(DeviceTreeContext* ctx)
{
    printf("-->\n");
    for(int i=0;i<ctx->scopeLevel;i++)
    {
        printf("%i '%s'\n", i, ctx->scopes[i]);
    }
    printf("<--\n");
}

static void pushScope(DeviceTreeContext* ctx, char* scope)
{
    assert(ctx->scopeLevel < MaxScope);
    ctx->scopes[ctx->scopeLevel] = scope;
    ctx->scopeLevel++;
    if(ctx->scopeLevel >= MaxScope)
    {
        assert(0);
    }
    //debugScope(ctx);
}

static void popScope(DeviceTreeContext* ctx)
{
    assert(ctx->scopeLevel < MaxScope);
    
    ctx->scopeLevel--;
    assert(ctx->scopeLevel >= 0);


    assert(ctx->scopes[ctx->scopeLevel] != NULL);
    free(ctx->scopes[ctx->scopeLevel]);
    ctx->scopes[ctx->scopeLevel] = NULL;
}

static int _startScope(AMLDecompiler* decomp,const ParserContext* context, const ACPIScope* scope)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);

    char* scopeName = AMLNameConstructNormalized(&scope->name);
    //printf("start scope '%s' level %i\n\n", scopeName, deviceContext->scopeLevel);
#if 0
    if(AMLNameHasPrefixRoot(&scope->name))
    {
        printf("New scope ROOT '%s'\n", scopeName);
    }
    else
    {   
        printf("New scope (%i)", deviceContext->scopeLevel);
        char *completePath = strdup("\\");
        size_t acc = strlen(completePath);
        for(int i=0;i<deviceContext->scopeLevel;i++)
        {

            acc += strlen(deviceContext->scopes[i]) + 1;

            completePath = realloc(completePath,acc);


            strcat(completePath, deviceContext->scopes[i]);
            strcat(completePath, ".");
        }
        completePath = realloc(completePath,acc + strlen(scopeName));
        strcat(completePath, scopeName);
        printf("'%s' (orig '%s')\n", completePath, scopeName);
    }
#endif
    pushScope(deviceContext, scopeName);

    //IONode* node = getNode(deviceContext, scopeName);


    return 0;
}
static int _endScope(AMLDecompiler* decomp,const ParserContext* context, const ACPIScope* scope)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);
    assert(deviceContext->scopeLevel);

    char* scopeName = AMLNameConstructNormalized(&scope->name);
    //printf("end scope '%s' level %i\n", scopeName, deviceContext->scopeLevel);
    free(scopeName);
    popScope(deviceContext);
    
    return 0;
}

static void getScopeNode(DeviceTreeContext* ctx, char*realName)
{

}


static int _startDevice(AMLDecompiler* decomp,const ParserContext* context, const ACPIDevice* dev)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);


    //printf("'%s'\n", realName);

    if(AMLNameHasPrefixRoot(&dev->name))
    {
        printf("\t-->Start root device: ");
        assert(deviceContext->rootNode);
        IONode*n = deviceContext->rootNode;

        for(uint8_t i=0;i<AMLNameCountSegments(&dev->name);i++)
        {
            char name[5] = "";
            AMLNameGetSegment(&dev->name, i, name);
//            printf("%s.", name);
            IONode* c = IONodeGetChildren(n, name);
            if(c == NULL)
            {
                c = malloc(sizeof(IONode));
                IONodeInit(c, strdup(name));
                IONodeAddChild(n, c);
            }
            n = c;

        }
//        printf("\n");
    }
    else 
    {
//        printf("\t-->Start non root device:\n");
        assert(deviceContext->rootNode);
        IONode*n = deviceContext->rootNode;

        for(int i=0;i<deviceContext->scopeLevel;i++)
        {
            char*name = deviceContext->scopes[i];

            if(i==0 && name[0] == '\\')
            {
                name +=1;
            }

            if(strchr(name, '.') != NULL)
            {
                char* dupName = strdup(name);
                char *ptr = strtok(dupName, ".");
                while(ptr != NULL)
                {                    
                    IONode* c = IONodeGetChildren(n, ptr);
                    if(c == NULL)
                    {
                        c = malloc(sizeof(IONode));
                        IONodeInit(c, strdup(ptr));
                        IONodeAddChild(n, c);
                    }
                    n = c;
                    
                    ptr = strtok(NULL, ".");
                }
                free(dupName);
            }
            else
            {
                IONode* c = IONodeGetChildren(n, name);
                if(c == NULL)
                {
                    c = malloc(sizeof(IONode));
                    IONodeInit(c, strdup(name));
                    IONodeAddChild(n, c);
                }
                n = c;
            }    

        }

        char* realName  = AMLNameConstructNormalized(&dev->name);
        if(IONodeGetChildren(n, realName) == NULL)
        {
            IONode* c = malloc(sizeof(IONode));
            IONodeInit(c, realName);
            IONodeAddChild(n, c);
        }
        else
        {
            free(realName);
        }
        
        

    }
    

    return 0;
}

static int _endDevice(AMLDecompiler* decomp,const ParserContext* context, const ACPIDevice* dev)
{
    return 0;
    char* realName  = AMLNameConstructNormalized(&dev->name);
    printf("\tEnd device '%s'\n", realName);
    free(realName);
    return 0;
}

static int _startName(AMLDecompiler* decomp,const ParserContext* context, const char* name)
{
//    printf("\tStart Name '%s'\n", name);
    return 0;
}
static int _onString(AMLDecompiler* decomp,const ParserContext* context, const char* string)
{
//    printf("\t\t String '%s'\n", string);
    return 0;
}

static int _onValue(AMLDecompiler* decomp,const ParserContext* context, uint64_t value)
{
    return 0;
    if(isEisaId(value))
    {
        char str[8] = "";
        getEisaidString(value, str);
        printf("\t\tEISAID value = '%s'  (0X%lX)\n", str, value);
    }
    else
    {
        printf("\t\tvalue = 0X%lX\n", value);
    }
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
    return 0;
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
    return 0;
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
//    printf("Start package\n");
    return 0;
}
static int _onPackageElement(AMLDecompiler* decomp,const ParserContext* context, const ACPIPackageElement* element)
{
//    printf("\ton package elt\n");
    return 0;
}
static int _endPackage(AMLDecompiler* decomp,const ParserContext* context, const ACPIPackage* package)
{
//    printf("end package\n");
    return 0;
}

static int _startResourceTemplate(AMLDecompiler* decomp, const ParserContext* context , size_t numItems )
{
//    printf("_startResourceTemplate\n");
    return 0;
}
static int _endResourceTemplate(AMLDecompiler* decomp, const ParserContext* context , size_t numItemsParsed, AMLParserError err)
{
//    printf("_endResourceTemplate\n");
    return 0;
}


static void walkDev(IONode* n, int indent)
{
    IONode* c = NULL;
    LL_FOREACH(n->children,c)
    {
        for(int i=0;i<indent;i++)
        {
            printf("\t");
        }
        printf("'%s'\n", c->name);

        size_t count = 0;
        IONode* tmp = NULL;
        LL_COUNT(c->children, tmp, count);
        if(count)
        {
            walkDev(c, indent+1);
        }
    }
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

    acpi_madt_t* madt = acpi_find_region(acpi, ACPI_MADT);
    assert(madt != NULL);
    KLOG_DEBUG("Got madt\n");


    size_t icsCount = 0;
    acpi_madt_ics_hdr_t* ics = acpi_madt_first_ics(madt);

    do
    {
        KLOG_DEBUG("ICS %zi: type %u len %u\n", icsCount, ics->type, ics->length);
        if(ics->type == ACPI_APIC_LOCAL)
        {
            const acpi_madt_local_apic_t* localApic = (const acpi_madt_local_apic_t*) ics;
            KLOG_DEBUG("Proc id %X ACPI id %u Flags %u\n", localApic->processor_id, localApic->apic_id, localApic->flags);

        }
        icsCount ++;
    } while (ics = acpi_madt_next_ics(madt, ics));
    KLOG_DEBUG("Got %zi ICS\n", icsCount);    



    acpi_header_t* dsdt = acpi_find_region(acpi, ACPI_DSDT);
    assert(dsdt != NULL);

    size_t acpiBufferSize = dsdt->length;


    AMLDecompiler decomp;
    AMLDecompilerInit(&decomp);
    AMLDecompilerUseDefaultCallbacks(&decomp);
    decomp.callbacks.startDevice = _startDevice;
    decomp.callbacks.endDevice = _endDevice;
    decomp.callbacks.startScope = _startScope;
    decomp.callbacks.endScope = _endScope;

    decomp.callbacks.onSmallItem = _onSmallItem;
    decomp.callbacks.onLargeItem = _onLargeItem;

    decomp.callbacks.startName = _startName;
    decomp.callbacks.onValue = _onValue;
    decomp.callbacks.onString = _onString;
    decomp.callbacks.onMethod = _onMethod;
    decomp.callbacks.startPackage = _startPackage;
    decomp.callbacks.onPackageElement = _onPackageElement;
    decomp.callbacks.endPackage = _endPackage;
    decomp.callbacks.startResourceTemplate = _startResourceTemplate;
    decomp.callbacks.endResourceTemplate = _endResourceTemplate;

    DeviceTreeContext  ctx;
    memset(&ctx, 0, sizeof(DeviceTreeContext));
    ctx.rootNode = &_rootNode;
    decomp.userData = &ctx;
    AMLParserError err = AMLDecompilerStart(&decomp, (const uint8_t*) dsdt, acpiBufferSize);
    
    assert( err == AMLParserError_None);

    printf("--------------------------\n");
    walkDev(&_rootNode, 0);
    printf("--------------------------\n");
}