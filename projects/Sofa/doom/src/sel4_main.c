/*
 * Copyright (c) 2015, Josef Mihalits
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "COPYING" for details.
 *
 */
#include <stdint.h>
#include <stddef.h>
#include <sel4/sel4.h>
#include <sel4/arch/bootinfo_types.h> // seL4_X86_BootInfo_VBE
#include "runtime.h"
#include <Sofa.h>
#include <files.h>
#include <dk.h>

static void* fb = NULL;

int main_ORIGINAL(int argc, char** argv);

/*
 *  @return: time since start (in ms)
 */
uint32_t sel4doom_get_current_time() 
{
    return 0;
}

void sel4doom_get_vbe(seL4_X86_BootInfo_VBE* mib) 
{
    //*mib = bootinfo2->vbeModeInfoBlock;
}

void *
sel4doom_get_framebuffer_vaddr() {
    return fb;
}

void*
sel4doom_load_file(const char* filename) 
{
    Printf("request to open '%s'\n", filename);
//    UNUSED unsigned long filesize;
//    return cpio_get_file(_cpio_archive, filename, &filesize);
    return NULL;
}

int main(int argc, char** argv)
{
    RuntimeInit2(argc, argv);
    argc -=2;
    argv = &argv[2];

    VFSClientInit();
    DKClientInit();
    Printf("Starting DOOOOOOOOOM\n");

#if 0
    DKDeviceHandle devHandle = DKClientGetDeviceNamed("Framebuffer", 4);
    if(devHandle == DKDeviceHandle_Invalid)
    {
        Printf("vga: invalid dev handle");
        return -1;
    }

    long ret = DKDeviceMMap(devHandle, 1);
    Printf("vga: ret = %li\n", ret);
    if(ret > 0)
    {
        fb = (void*) ret;
    }
    else
    {
        return -1;
    }
#endif
    Printf("Test alloc\n");

    void* p = malloc(6*1024*1024);
    if(p)
    {
        Printf("Test alloc ok\n");
        return 0;
    }
    Printf("Test alloc error\n");
    return 0;

    
    return main_ORIGINAL(argc, argv);
}