/*
 * Copyright (c) 2015, Josef Mihalits
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "COPYING" for details.
 *
 */

#ifndef SEL4_DOOM_H_
#define SEL4_DOOM_H_

#include <stdint.h>
#include <sel4/sel4.h>
#include <sel4/arch/bootinfo_types.h> // seL4_X86_BootInfo_VBE


void*
sel4doom_get_framebuffer_vaddr();


void
sel4doom_get_vbe(seL4_X86_BootInfo_VBE* mib);


int
sel4doom_keyboard_poll_keyevent(int16_t* vkey);


unsigned int
sel4doom_get_current_time();


void*
sel4doom_load_file(const char* filename);


void
sel4doom_set_image(int imgId);

#endif /* SEL4_DOOM_H_ */
