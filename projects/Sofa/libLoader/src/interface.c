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
#include <loader.h>
#include <stddef.h>
#include "lib-support.h"

void *SF_DLOpen(const char *filename)
{
    return DLoader.load(filename);
}


void* SF_DLGetFunction(void* handle, int num)
{
    void **func_table = DLoader.get_info(handle);
    if(func_table)
    {
        return func_table[num];
    }
    return NULL;
}

int SF_DLClose(void* handler)
{
    return -1;
}