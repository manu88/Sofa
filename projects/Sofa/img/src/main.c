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
#include "runtime.h"
#include <dk.h>
#include <Sofa.h>
#include <files.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


static void* loadFile(const char* filename)
{
    int fd = open(filename, O_RDONLY);

    uint8_t *data = NULL;
    int done = 0;
    size_t sizeAccum = 0;
    size_t posToCopy = 0;

    #define READ_SIZE 4096
    char buf[READ_SIZE] = "";
    while (!done)
    {
        ssize_t ret = read(fd, buf, READ_SIZE);// VFSRead(&caller->_base, &file, buf, READ_SIZE, NULL);

        if(ret > 0)
        {
            sizeAccum+= ret;
            data = realloc(data, sizeAccum);
            assert(data);
            memcpy(data + posToCopy, buf, ret);
            posToCopy += ret;
        }
        else
        {
            if(ret < 0)
            {
                if(data)
                {
                    free(data);
                }
                close(fd);
                return NULL;
            }
            break;
        }
    }


    close(fd);

    return data;
}



static void putPixel(void* fb, int x, int y, uint8_t r,  uint8_t g, uint8_t b)
{
    if(x>= 650 || x < 0)
    {
        return;
    }
    if(y>= 480 || y < 0)
    {
        return;
    }

    unsigned int coord_factor = 4;
    size_t len = 3;
    char* target = ((char *)fb) + (y * 640 + x) * coord_factor;

    target[0] = r;
    target[1] = g;
    target[2] = b;
}

static void clear(void* fb, int x, int y, int w, int h)
{
    for(int xx=x;xx<x+w;xx++)
    {
        for(int yy=y;yy<y+h;yy++)
        {
            putPixel(fb, xx, yy, 0,0,0);
        }
    }
}

static int imgx = 0;
static int imgy = 0;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;


static Pixel* pixels = NULL;

static void displayPPM(void*fb, void* img, int startX, int startY) 
{
    if(pixels == NULL)
    {
        //we do not handle comments in the header
        int n = sscanf(img, "P6\n%d %d\n255\n", &imgx, &imgy);
        assert (n == 2);
        assert(0 < imgx);
        assert(imgx < 256);
        assert(0 < imgy);
        assert(imgy < 256);
        //assert (n == 2 && imgx > 0 && imgy > 0);

        // find first pixel; header and data are separated by "\n255\n"
        uint8_t* src = (uint8_t*) strstr(img, "\n255\n");
        assert(src);
        // skip over separator
        src += 5;

        pixels = malloc(sizeof(Pixel)*imgx*imgy);
        assert(pixels);
        int i=0;
        for (int y = 0; y < imgy; y++) 
        {
            for (int x = 0; x < imgx; x++) 
            {
                pixels[i].r = *src++;
                pixels[i].g = *src++;
                pixels[i].b = *src++;

                i++;
            }
        }
    }

    for(int i=0;i<imgx*imgy;i++)
    {
        int x = i % imgx;
        int y = i / imgx;
        putPixel(fb, startX + x, startY + y, pixels[i].r, pixels[i].g, pixels[i].b);
    }
}

int main(int argc, char *argv[])
{   
    RuntimeInit2(argc, argv);
    argc -=2;
    argv = &argv[2];
    VFSClientInit();
    DKClientInit();
    if(argc < 2)
    {
        Printf("usage: img fileToDisplay\n");
        return 1;
    }
    Printf("Open ppm file '%s'\n", argv[1]);


    DKDeviceHandle devHandle = DKClientGetDeviceNamed("Framebuffer", 4);
    if(devHandle == DKDeviceHandle_Invalid)
    {
        Printf("vga: invalid dev handle");
        return 2;
    }

    long ret = DKDeviceMMap(devHandle, 1);

    if(ret > 0)
    {
        void* fb = (void*) ret;

        void * img = loadFile(argv[1]);
        if(img == NULL)
        {
            return 3;
        }

        int x = 0;
        int y = 0;

        int lastX = -1;
        int lastY = -1;

        while (1)
        {
            clear(fb, lastX, lastY, imgx, imgy);
            displayPPM(fb, img, x, y);

            lastX = x;
            lastY = y;

            x++;
            y++;

            if(x>640)
            {
                x = 0;
            }
            if(y>480)
            {
                y = 0;
            }

            SFSleep(40);
        }
        
        
    }


    return 0;
}

