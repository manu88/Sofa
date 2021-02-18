// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff.
//
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <assert.h>


#include "m_swap.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"
#include "sel4_doom.h"
//#include "sel4.local/libplatsupport/keyboard_vkey.h"

/* the current color palette in 32bit format as used by frame buffer */
static uint32_t sel4doom_colors32[256];

/* the current color table: one byte for red, one for green, one for blue */
static uint8_t sel4doom_colors[256][3];

/* VBE mode info */
static seL4_X86_BootInfo_VBE mib;

/* base address of frame buffer (virtual address) */
static uint32_t* sel4doom_fb = NULL;

/* current ID of image displayed during game play; -1 = no image */
static int sel4doom_imgId = -1;

/* This version of DOOM uses 320 pixels per row. Because of "blocky" mode
 * (multiply * 320) pixels are displayed per row. The number of pixels on the real
 * screen is determined by the current graphics mode and is given by mib.xRes.
 * If mib.xRes is not a multiple of 320, then a black margin area remains
 * between the content area and the screen area. row_offset is the width of this
 * margin area and the number of pixel rows that are skipped in "blocky" mode.
 */
static uint32_t row_offset = 0;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static uint32_t	multiply = 1;


/*
 * Set all pixels to black.
 */
static void
sel4doom_clear_screen() {
    const size_t size = mib.vbeModeInfoBlock.vbe12_part1.yRes * mib.vbeModeInfoBlock.vbe30.linBytesPerScanLine;
    for (int i = 0; i < size / 4; i++) {
        sel4doom_fb[i] = 0;
    }
}

/*
 * Load and preprocess a ppm image.
 */
static uint8_t*
sel4doom_get_ppm(int imgId) {
    char filename[20];
    sprintf(filename, "logo%d.ppm", imgId);
    void * img = sel4doom_load_file(filename);
    assert(img);

    int imgx = 0;  // image width (in pixel)
    int imgy = 0;  // image height
    //we do not handle comments in the header
    int n = sscanf(img, "P6\n%d %d\n255\n", &imgx, &imgy);
    assert (n == 2 && 0 < imgx && imgx < 256 && 0 < imgy && imgy < 256);
    //assert (n == 2 && imgx > 0 && imgy > 0);

    // find first pixel; header and data are separated by "\n255\n"
    uint8_t* src = (uint8_t*) strstr(img, "\n255\n");
    assert(src);
    // skip over separator
    src += 5;
    uint8_t* cache = malloc (imgx * imgy + 2);
    cache[0] = imgx;
    cache[1] = imgy;
    uint8_t* dst = cache + 2;

    for (int y = 0; y < imgy; y++) {
        for (int x = 0; x < imgx; x++) {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;

            if (r > 220 && g > 220 && b > 220) {
                // treat this pixel as transparent; it will not be displayed
                *dst++ = 255;
            } else {
                // find closest matching color (think 3d color space)
                int minidx = 0;
                int mindist = 0;
                for (int i = 0; i < 256; i++) {
                    int dist = abs(r - sel4doom_colors[i][0])
                             + abs(g - sel4doom_colors[i][1])
                             + abs(b - sel4doom_colors[i][2]);
                    if (i == 0 || dist < mindist) {
                        minidx = i;
                        mindist = dist;
                    }
                }
                *dst++ = minidx;
            }
        }
    }
    return cache;
}


/*
 * Display a ppm image file at upper right corner of output screen.
 */
static void
sel4doom_diplay_ppm (int imgId) {
    static uint8_t* img_cache[] = {NULL, NULL};
    assert(imgId == 0 || imgId == 1);

    /* load image if not in cache */
    uint8_t* img = img_cache[imgId];
    if (img == NULL) {
        img = img_cache[imgId] = sel4doom_get_ppm(imgId);
    }
    /* width and height of the image */
    uint32_t imgx = *(img++);
    uint32_t imgy = *(img++);
    /* we don't write directly to the frame buffer but to screens[0] */
    uint8_t* screen = screens[0];
    /* the location in screens[0] where image will appear */
    uint32_t startx = SCREENWIDTH - imgx;
    uint32_t starty = 0;

    /* The code here may be sub-optimal as we only copy one byte at the time,
     * but the code is not expected to be active anyway...
     */
    for (int y = 0; y < imgy; y++) {
        uint8_t* dst = screen + (starty + y) * SCREENWIDTH + startx;
        /* copy one row */
        for (int x = 0; x < imgx; x++, dst++, img++) {
            /* skip of the pixels marked transparent */
            if (*img != 255) {
                *dst = *img;
            }
        }
    }
}


/*
 * Set / unset ID of currently displayed image.
 */
void
sel4doom_set_image(int imgId) {
    if (sel4doom_imgId == imgId) {
        sel4doom_imgId = -1;
    } else {
        sel4doom_imgId = imgId;
    }
}


static int
sel4doom_poll_event(event_t* event) {
    return 0;
#if 0
    int16_t vkey;
    int pressed = sel4doom_keyboard_poll_keyevent(&vkey);
    if (vkey == -1) {
        //no pending events
        return 0;
    }

    event->type = pressed ? ev_keydown : ev_keyup;

    switch(vkey) {
    case VK_LEFT:
        event->data1 = KEY_LEFTARROW;
        return 1;
    case VK_UP:
        event->data1 = KEY_UPARROW;
        return 1;
    case VK_RIGHT:
        event->data1 = KEY_RIGHTARROW;
        return 1;
    case VK_DOWN:
        event->data1 = KEY_DOWNARROW;
        return 1;
    case VK_SNAPSHOT: // print screen key
        // Hack: We filter out this event (by returning 0).
        // At least in VirtualBox, every cursor key event is accompanied
        // by this key event.
        return 0;
    case VK_BACK:
        event->data1 = KEY_BACKSPACE;
        return 1;
    case VK_LSHIFT: //left shift
        event->data1 = KEY_RSHIFT;
        return 1;
    case VK_RSHIFT: //right shift
        event->data1 = KEY_RSHIFT;
        return 1;
    case VK_LCONTROL: //left control
        event->data1 = KEY_RCTRL;
        return 1;
    case VK_RCONTROL:
        event->data1 = KEY_RCTRL;
        return 1;
    case VK_MENU: // left and right alt
        event->data1 = KEY_RALT;
        return 1;
    case VK_LMENU:
        event->data1 = KEY_RALT;
        return 1;
    case VK_RMENU:
        event->data1 = KEY_RALT;
        return 1;
    case VK_OEM_COMMA: // ',' key
        event->data1 = 44; // ascii code of ',' (default key for strafe left)
        return 1;
    case VK_OEM_PERIOD: // '.' key
        event->data1 = 46; // ascii code of '.' (default key for strafe right)
        return 1;
    case VK_F1:
        event->data1 = KEY_F1;
        return 1;
    case VK_F2:
        event->data1 = KEY_F2;
        return 1;
    case VK_F3:
        event->data1 = KEY_F3;
        return 1;
    case VK_F4:
        event->data1 = KEY_F4;
        return 1;
    case VK_F5:
        event->data1 = KEY_F5;
        return 1;
    case VK_F6:
        event->data1 = KEY_F6;
        return 1;
    case VK_F7:
        event->data1 = KEY_F7;
        return 1;
    case VK_F8:
        event->data1 = KEY_F8;
        return 1;
    case VK_F9:
        event->data1 = KEY_F9;
        return 1;
    case VK_F10:
        event->data1 = KEY_F10;
        return 1;
    case VK_F11:
        event->data1 = KEY_F11;
        return 1;
    case VK_F12:
        event->data1 = KEY_F12;
        return 1;
    case VK_OEM_PLUS:
        event->data1 = KEY_EQUALS;
        return 1;
    case VK_OEM_MINUS:
        event->data1 = KEY_MINUS;
        return 1;
    case VK_PAUSE:
        event->data1 = KEY_PAUSE;
        return 1;
    case VK_NUMPAD4:
        event->data1 = KEY_LEFTARROW;
        return 1;
    case VK_NUMPAD8:
        event->data1 = KEY_UPARROW;
        return 1;
    case VK_NUMPAD6:
        event->data1 = KEY_RIGHTARROW;
        return 1;
    case VK_NUMPAD5:
        event->data1 = KEY_DOWNARROW;
        return 1;
    case VK_NUMPAD2:
        event->data1 = KEY_DOWNARROW;
        return 1;

    /* Some extra mappings. */
    case VK_X:
        event->data1 = KEY_RALT;
        return 1;
    case VK_Z:
        event->data1 = KEY_RCTRL;
        return 1;
    }

    if (65 <= vkey && vkey <= 90) {
        //ASCII shift: upper case chars to lower case chars
        //IDKFA :->
        event->data1 = vkey + 32;
        return 1;
    }

    //covers ESC, Enter, digits 1-0
    event->data1 = vkey;
    return 1;
#endif
}


void I_ShutdownGraphics(void)
{
    sel4doom_clear_screen();
}


//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?
}


//
// I_StartTic
//
void I_StartTic(void)
{
    event_t event;
    while (sel4doom_poll_event(&event)) {
        D_PostEvent(&event);
    }
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}


//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    // draws little dots on the bottom of the screen (frame rate)
    if (devparm)
    {
        static int lasttic = 0;
        int curtic = I_GetTime();
        int tics = curtic - lasttic;
        lasttic = curtic;
        if (tics > 20) {
            tics = 20;
        }

        int i;
        for (i=0 ; i<tics*2 ; i+=2) {
            screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
        }
        for ( ; i<20*2 ; i+=2) {
            screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
        }
    }
    if (sel4doom_imgId != -1) {
        sel4doom_diplay_ppm(sel4doom_imgId);
    }
    // ------------------------
    if (multiply == 1)
    {
        unsigned int *src = (unsigned int *) screens[0];
        unsigned int *dst = sel4doom_fb;
        for (int y = SCREENHEIGHT; y; y--) {
            for (int x = SCREENWIDTH; x; x -= 4) {
                /* We process four pixels per iteration. */
                unsigned int fourpix = *src++;

                //first "src" pixel
                *dst++ = sel4doom_colors32[fourpix & 0xff];
                //second "src" pixel
                *dst++ = sel4doom_colors32[(fourpix >> 8) & 0xff];
                //third "src" pixel
                *dst++ = sel4doom_colors32[(fourpix >> 16) & 0xff];
                //fourth "src" pixel
                *dst++ = sel4doom_colors32[fourpix >> 24];
            }
            dst += row_offset;
        }
        return;
    }
    // ------------------------
    if (multiply == 2)
    {
        /* pointer into source screen */
        unsigned int *src = (unsigned int *) (screens[0]);

        /*indices into frame buffer, one per row */
        int dst[2] = {0, mib.vbeModeInfoBlock.vbe12_part1.xRes};

        for (int y = SCREENHEIGHT; y; y--) {
            for (int x = SCREENWIDTH; x; x -= 4) {
                /* We process four "src" pixels per iteration
                 * and for every source pixel, we write out 4 pixels to "dst".
                 */
                unsigned fourpix = *src++;

                /* 32 bit RGB value */
                unsigned int p;
                //first "src" pixel
                p = sel4doom_colors32[fourpix & 0xff];
                sel4doom_fb[dst[0]++] = p;  //top left
                sel4doom_fb[dst[0]++] = p;  //top right
                sel4doom_fb[dst[1]++] = p;  //bottom left
                sel4doom_fb[dst[1]++] = p;  //bottom right

                //second "src" pixel
                p = sel4doom_colors32[(fourpix >> 8) & 0xff];
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;

                //third "src" pixel
                p = sel4doom_colors32[(fourpix >> 16) & 0xff];
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;

                //fourth "src" pixel
                p = sel4doom_colors32[fourpix >> 24];
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;
            }
            dst[0] += row_offset;
            dst[1] += row_offset;
        }
        return;
    }
    if (multiply == 3)
    {
        /* pointer into source screen */
        unsigned int *src = (unsigned int *) (screens[0]);

        /*start indices into frame buffer, one per row */
        int dst[3] = {0, mib.vbeModeInfoBlock.vbe12_part1.xRes, mib.vbeModeInfoBlock.vbe12_part1.xRes + mib.vbeModeInfoBlock.vbe12_part1.xRes};

        for (int y = SCREENHEIGHT; y; y--) {
            for (int x = SCREENWIDTH; x; x -= 4) {
                /* We process four "src" pixels per iteration
                 * and for every source pixel, we write out 9 pixels to "dst".
                 */
                unsigned fourpix = *src++;

                /* 32 bit RGB value */
                unsigned int p;
                //first "src" pixel
                p = sel4doom_colors32[fourpix & 0xff];
                sel4doom_fb[dst[0]++] = p;  //top row, left
                sel4doom_fb[dst[0]++] = p;  //top row, middle
                sel4doom_fb[dst[0]++] = p;  //top row, right
                sel4doom_fb[dst[1]++] = p;  //middle row, left
                sel4doom_fb[dst[1]++] = p;  //middle row, middle
                sel4doom_fb[dst[1]++] = p;  //middle row, right
                sel4doom_fb[dst[2]++] = p;  //bottom row, left
                sel4doom_fb[dst[2]++] = p;  //bottom row, middle
                sel4doom_fb[dst[2]++] = p;  //bottom row, right

                //second "src" pixel
                p = sel4doom_colors32[(fourpix >> 8) & 0xff];
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[2]++] = p;
                sel4doom_fb[dst[2]++] = p;
                sel4doom_fb[dst[2]++] = p;

                //third "src" pixel
                p = sel4doom_colors32[(fourpix >> 16) & 0xff];
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[2]++] = p;
                sel4doom_fb[dst[2]++] = p;
                sel4doom_fb[dst[2]++] = p;

                //fourth "src" pixel
                p = sel4doom_colors32[fourpix >> 24];
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[0]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[1]++] = p;
                sel4doom_fb[dst[2]++] = p;
                sel4doom_fb[dst[2]++] = p;
                sel4doom_fb[dst[2]++] = p;
            }

            //each dst moves two pixel rows forward
            dst[0] += row_offset;
            dst[1] += row_offset;
            dst[2] += row_offset;
        }
        return;
    }
    I_Error("Unsupported 'multiply' factor.");
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    for (int i = 0; i < 256; i++) {
        byte r = gammatable[usegamma][*palette++];
        byte g = gammatable[usegamma][*palette++];
        byte b = gammatable[usegamma][*palette++];
        sel4doom_colors[i][0] = r;
        sel4doom_colors[i][1] = g;
        sel4doom_colors[i][2] = b;
        sel4doom_colors32[i] =
                      (r << mib.vbeModeInfoBlock.vbe30.linRedOff)
                    | (g << mib.vbeModeInfoBlock.vbe30.linGreenOff)
                    | (b << mib.vbeModeInfoBlock.vbe30.linBlueOff);
    }
}


void I_InitGraphics(void) {
    sel4doom_fb = sel4doom_get_framebuffer_vaddr();
    if (sel4doom_fb == NULL) {
         I_Error("No frame buffer");
    }

    // Graphics changes are "collected" in screens[0] and then written out
    // to the real screen in one go in I_FinishUpdate().
    screens[0] = (unsigned char *) malloc (SCREENWIDTH * SCREENHEIGHT);
    if (screens[0] == NULL) {
        I_Error("Couldn't allocate screen memory");
    }

    sel4doom_get_vbe(&mib);
    printf("seL4: I_InitGraphics: xRes=%d yRes=%d bpp=%d\n",
                mib.vbeModeInfoBlock.vbe12_part1.xRes, mib.vbeModeInfoBlock.vbe12_part1.yRes, mib.vbeModeInfoBlock.vbe12_part1.bitsPerPixel);

    // select scaling factor for output (a.k.a. blocky mode)
    if (M_CheckParm("-1")) {
        multiply = 1;
        printf("seL4: I_InitGraphics: -1 option detected\n");
    } else if (M_CheckParm("-2")) {
        multiply = 2;
        printf("seL4: I_InitGraphics: -2 option detected\n");
    } else if (M_CheckParm("-3")) {
        multiply = 3;
        printf("seL4: I_InitGraphics: -3 option detected\n");
    } else if (SCREENWIDTH * 3 <= mib.vbeModeInfoBlock.vbe12_part1.xRes && SCREENHEIGHT * 3 <= mib.vbeModeInfoBlock.vbe12_part1.yRes) {
        multiply = 3;
    } else if (SCREENWIDTH * 2 <= mib.vbeModeInfoBlock.vbe12_part1.xRes && SCREENHEIGHT * 2 <= mib.vbeModeInfoBlock.vbe12_part1.yRes) {
        multiply = 2;
    } else {
        multiply = 1;
    }
    printf("seL4: I_InitGraphics: setting multiply=%d\n", multiply);

    row_offset = multiply * (mib.vbeModeInfoBlock.vbe12_part1.xRes - SCREENWIDTH);
    sel4doom_clear_screen();
}
