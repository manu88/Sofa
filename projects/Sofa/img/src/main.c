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
#include "Image.h"
#include "runtime.h"
#include <dk.h>
#include <Sofa.h>
#include <files.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


static void* sel4doom_load_file(const char* filename)
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

    unsigned int coord_factor = 4;
    size_t len = 3;
    char* target = ((char *)fb) + (y * 640 + x) * coord_factor;

    target[0] = r;
    target[1] = g;
    target[2] = b;
}

static uint8_t*
sel4doom_get_ppm(void*fb, const char* filename) 
{

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

    for (int y = 0; y < imgy; y++) 
    {
        for (int x = 0; x < imgx; x++) 
        {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;

           putPixel(fb, x,y, r,g, b);
        }
    }
    free(img);
    return NULL;
}

#define PPMREADBUFLEN 256
image get_ppm(FILE *pf)
{
        char buf[PPMREADBUFLEN], *t;
        image img;
        unsigned int w, h, d;
        int r;
 
        if (pf == NULL) 
        {
            return NULL;
        }
        t = fgets(buf, PPMREADBUFLEN, pf);
        /* the code fails if the white space following "P6" is not '\n' */
        
        if ( (t == NULL) || ( strncmp(buf, "P6\n", 3) != 0 ) ) 
        {
            return NULL;
        }
        do
        { /* Px formats can have # comments after first line */
           t = fgets(buf, PPMREADBUFLEN, pf);

           if ( t == NULL )
           { 
                return NULL;
           }
        }
        while ( strncmp(buf, "#", 1) == 0 );
        r = sscanf(buf, "%u %u", &w, &h);
        if ( r < 2 ) 
        {
            return NULL;
        }
 
        r = fscanf(pf, "%u", &d);
        if ( (r < 1) || ( d != 255 ) ) 
        {
            return NULL;
        }
        fseek(pf, 1, SEEK_CUR); /* skip one byte, should be whitespace */
 
        img = alloc_img(w, h);

        if ( img != NULL )
        {
            size_t rd = fread(img->buf, sizeof(pixel), w*h, pf);
            if ( rd < w*h )
            {
               free_img(img);
               return NULL;
            }
            return img;
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
        return -1;
    }

    long ret = DKDeviceMMap(devHandle, 1);

    if(ret > 0)
    {
        void* fb = (void*) ret;

        sel4doom_get_ppm(fb, argv[1]);
    }


    return 0;
}

