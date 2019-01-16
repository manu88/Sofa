/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "Sofa.h"

#include "Bootstrap.h"

#include "KObject/KObject.h"

#include "SystemTree.h"
#include "DriverKit.h"
#include "ProcessTable.h"
#include "FileSystem.h"
#include "Modules/VGADevice.h"

#ifndef SOFA_TESTS_ONLY

#include <platsupport/plat/acpi/acpi.h>
#include <sel4platsupport/io.h>

#else
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
static uint8_t* readAndFillBuffer(const char* fromFile , size_t* bufSize)
{
    
    int fd = open(fromFile, O_RDONLY);
    
    uint8_t c = 0;
    
    uint8_t* ptr = NULL;
    
    size_t size = 0;
    
    while(  read(fd, &c, 1) >0)
    {
        uint8_t* lastP = ptr;
        ptr =  realloc(ptr, ++size);
        
        if (!ptr)
        {
            free(lastP);
            *bufSize = 0;
            return NULL;
        }
        ptr[size-1] = c;
        
    }
    
    *bufSize = size;
    
    return ptr;
}
#endif


VGADevice _vgaDev;

/*
 -EarlySystemInit : SeL4 and Hardware initialization
 -BaseSystemInit  : Sofa System init
 -LateSystemInit  : Drivers & userland bootstrap
 */


static KernelTaskContext context = { 0 };

// Mostly Sel4 boostrap
static OSError EarlySystemInit()
{
    printf("EarlySystemInit\n");
    memset(&context , 0 , sizeof(KernelTaskContext) );
    
#ifndef SOFA_TESTS_ONLY
    zf_log_set_tag_prefix("kernel_task");
#endif

    int error = 0;

    context.info = platsupport_get_bootinfo();
#ifndef SOFA_TESTS_ONLY
    ZF_LOGF_IF(context.info == NULL, "Failed to get bootinfo.");
#endif
    
    error = bootstrapSystem( &context);
    
#ifndef SOFA_TESTS_ONLY
    ZF_LOGF_IFERR(error, "Failed to bootstrap system.\n");
#endif

    assert(error == 0);

    return error;
}

static OSError BaseSystemInit()
{
    printf("BaseSystemInit\n");

    assert( SystemTreeInit() == OSError_None);
    assert( SystemTreeGetRoot() && SystemTreeGetRoot()->parent == SystemTreeGetRoot());
    
    
    assert( DriverKitInit() == OSError_None);
    assert( SystemTreeAddObject(DriverKitGetObject() ) == OSError_None );
    assert(DriverKitGetObject()->parent == SystemTreeGetRoot() );
    
    uint8_t *acpiBuffer = NULL;
    size_t acpiBufferSize = 0;
#ifdef SOFA_TESTS_ONLY
    acpiBuffer = readAndFillBuffer("qemu-dsdt.aml", &acpiBufferSize);
#else
    ps_io_mapper_t io_mapper;
    error =  sel4platsupport_new_io_mapper(context.vspace, context.vka, &io_mapper);
    assert(error == 0);
    
    acpi_t* acpi = acpi_init(io_mapper);
    
    assert(acpi != NULL);
    
    acpi_dsdt_t* dsdt = acpi_find_region(acpi, ACPI_DSDT);
    assert(dsdt != NULL);
    
    acpiBuffer = dsdt->definition_block;
    acpiBufferSize = dsdt->header.length;
#endif
    
    assert(acpiBuffer);
    if(DriverKitContructDeviceTree(acpiBuffer , acpiBufferSize) != OSError_None)
    {
        return OSError_Some;
    }
    
    assert( ProcessTableInit() == OSError_None);
    assert( SystemTreeAddObject(ProcessTableGetObject() ) == OSError_None );
    assert(ProcessTableGetObject()->parent == SystemTreeGetRoot() );
    
    assert( FileSystemInit() == OSError_None);
    assert( SystemTreeAddObject(FileSystemGetObject() ) == OSError_None );
    assert(FileSystemGetObject()->parent == SystemTreeGetRoot() );
    
    return OSError_None;
}

static OSError LateSystemInit()
{
    printf("LateSystemInit\n");
    
    /*
    OSError err = VGADeviceInit(&_vgaDev);
    
    if (err == OSError_None)
    {
        err = DriverKitRegisterDevice(&_vgaDev.obj);
    }
    */
    return OSError_None;
}

int main(int argc, const char * argv[])
{
    assert(EarlySystemInit() == OSError_None );
    assert(BaseSystemInit()  == OSError_None );
    assert(LateSystemInit()  == OSError_None );
    
    
    
//    SystemTreeDump();
    DriverKitDump();
    return 0;
}
