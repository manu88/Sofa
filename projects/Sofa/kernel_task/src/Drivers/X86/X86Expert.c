/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "X86Expert.h"
#include "Log.h"
#include "Environ.h"
#include "DeviceTree.h"
#include "IONode.h"

#include "PCI.h"

#include <platsupport/plat/acpi/acpi.h>
#include <platsupport/plat/acpi/tables/madt.h>
#include <AMLDecompiler.h>
#include <EISAID.h>

static void ACPIParse(IONode* root);

int PlatformExpertInit()
{
    KLOG_INFO("PlatformExpertInit for X86_64\n");
    return 0;
}

int PlatformExpertConstructTree(IONode *root)
{
    ACPIParse(root);

    return 0;    
}




/***** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** */
// ACPI Parser
/***** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

typedef struct
{
    IONode *rootNode;
    
#define MaxScope 64
    char *scopes[64];
    int scopeLevel;

    IONode* currentNode;

    char const* currentName;
} DeviceTreeContext;


static void debugScope(DeviceTreeContext* ctx)
{
    KLOG_DEBUG("-->\n");
    for(int i=0;i<ctx->scopeLevel;i++)
    {
        KLOG_DEBUG("%i '%s'\n", i, ctx->scopes[i]);
    }
    KLOG_DEBUG("<--\n");
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

    pushScope(deviceContext, scopeName);


    return 0;
}
static int _endScope(AMLDecompiler* decomp,const ParserContext* context, const ACPIScope* scope)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);
    assert(deviceContext->scopeLevel);
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

    if(deviceContext->currentNode != NULL)
    {
        char* n = AMLNameConstructNormalized(&dev->name);
        KLOG_DEBUG("StartDevice Warning: currentNode should be NULL for '%s'\n", n);
        KLOG_DEBUG("Instead is '%s'\n", deviceContext->currentNode->name);
        debugScope(deviceContext);        
    }
//    assert(deviceContext->currentNode == NULL);

    IONode*n = deviceContext->rootNode;

    if(AMLNameHasPrefixRoot(&dev->name))
    {
        assert(deviceContext->rootNode);

        for(uint8_t i=0;i<AMLNameCountSegments(&dev->name);i++)
        {
            char name[5] = "";
            AMLNameGetSegment(&dev->name, i, name);
            IONode* c = IONodeGetChildren(n, name);
            if(c == NULL)
            {
                c = malloc(sizeof(IONode));
                
                assert(strchr(name, '.') == NULL);
                IONodeInit(c, strdup(name));
                IONodeAddChild(n, c);
            }
            n = c;

        }
    }
    else 
    {
        assert(deviceContext->rootNode);

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
                        assert(strchr(ptr, ".") == NULL);

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
                    assert(strchr(name, ".") == NULL);
                    IONodeInit(c, strdup(name));
                    IONodeAddChild(n, c);
                }
                n = c;
            }
        }
        char* realName  = AMLNameConstructNormalized(&dev->name);

        if(IONodeGetChildren(n, realName) == NULL)
        {
            if(strchr(realName, '.') != NULL)
            {
                KLOG_DEBUG("Not expecting '.' in '%s'\n", realName);
            }
            assert(strchr(realName, '.') == NULL);
            IONode* c = malloc(sizeof(IONode));

            IONodeInit(c, realName);
            IONodeAddChild(n, c);
            n = c;
        }
        else
        {
            free(realName);
        }
    }
    deviceContext->currentNode = n;
    
    assert(deviceContext->currentNode);

    return 0;
}

static int _endDevice(AMLDecompiler* decomp,const ParserContext* context, const ACPIDevice* dev)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);


    if(deviceContext->currentNode == NULL)
    {
        char* n = AMLNameConstructNormalized(&dev->name);
        KLOG_DEBUG("StartDevice Warning: currentNode should NOT be NULL for '%s'\n", n);
        debugScope(deviceContext);        
    }

//    assert(deviceContext->currentNode);
    deviceContext->currentNode = NULL;
    return 0;
}

static int _startName(AMLDecompiler* decomp,const ParserContext* context, const char* name)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);
    if(deviceContext->currentNode)
    {
        deviceContext->currentName = name;
    }

//    printf("\tStart Name '%s'\n", name);
    return 0;
}
static int _onString(AMLDecompiler* decomp,const ParserContext* context, const char* string)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);

    if(deviceContext->currentNode && deviceContext->currentName)
    {

        if (strcmp("_HID", deviceContext->currentName) == 0)
        {
            deviceContext->currentNode->hid.type = IOVariantType_STRING;
            deviceContext->currentNode->hid.value.s = string;
            // = IOValueString(string);
        }
    }
/*
    if(strcmp(string, "QEMU0002") == 0)
    {
        printf("Got QEMU0002 for name '%s' '%s'\n",deviceContext->currentNode->name, deviceContext->currentName);
    }
*/
    //printf("\t\t String '%s'\n", string);
    return 0;
}

static int _onValue(AMLDecompiler* decomp,const ParserContext* context, uint64_t value)
{
    DeviceTreeContext* deviceContext = decomp->userData;
    assert(deviceContext);

    if(deviceContext->currentNode && deviceContext->currentName)
    {
//        printf("Got value %X for name '%s' in device '%s'\n", value, deviceContext->currentName, deviceContext->currentNode->name);
        if (strcmp("_HID", deviceContext->currentName) == 0)
        {
            //deviceContext->currentNode->hid = IOValueUINT64(value);
            deviceContext->currentNode->hid.type = IOVariantType_UINT64;
            deviceContext->currentNode->hid.value.v = value;
        }
    }
    deviceContext->currentName = NULL;
    return 0;
    if(isEisaId(value))
    {
        char str[8] = "";
        getEisaidString(value, str);
        KLOG_DEBUG("\t\tEISAID value = '%s'  (0X%lX)\n", str, value);
    }
    else
    {
        KLOG_DEBUG("\t\tvalue = 0X%lX\n", value);
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


static int probeNode(IONode* n)
{
    if(n->hid.value.v == 0)
    {
//        KLOG_DEBUG("probeNode: skip '%s' because no HID\n", n->name);
        return -EINVAL;
    }

    // eisaID should be 7 chars as ACPI spec, BUT QEMU's FWCF HID's is 8...
    char eisaID[9] = "";
    if(IONodeGetEISAID(n, eisaID) != 0)
    {
        KLOG_DEBUG("probeNode: skip '0X%lX' because not an EISAID\n", n->hid.value.v);
        return -EINVAL;
    }
    
    KLOG_INFO("-->Probing '%s'\n", eisaID);

    if(strcmp(eisaID, "PNP0A03") == 0)
    {
        return PCIDriverInit(n);
    }
    return -EINVAL;
}


static void walkDev(IONode* n, int indent)
{
    IONode* c = NULL;
    IONodeForEachChildren(n, c)
    {
        probeNode(c);
        walkDev(c, indent+1);
    }
}

static void ACPIParse(IONode *root)
{
    KernelTaskContext* env = getKernelTaskContext();
    ps_io_mapper_t io_mapper;
    vspace_t* mainVSpace = getMainVSpace();

    int error =  sel4platsupport_new_io_mapper(mainVSpace, env->vka, &io_mapper);
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
    ctx.rootNode = root;
    decomp.userData = &ctx;
    AMLParserError err = AMLDecompilerStart(&decomp, (const uint8_t*) dsdt, acpiBufferSize);
    
    assert( err == AMLParserError_None);

    printf("--------------------------\n");
    walkDev(root, 0);
    printf("--------------------------\n");
}