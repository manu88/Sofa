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
#include <vka/object.h>
#include "Allocator.h"
#include "utlist.h"


static size_t _kmallocatedMem;

extern uint8_t _env_set;
void *kmalloc(size_t size)
{
    assert(_env_set);
    return malloc(size);
}
void kfree(void *ptr)
{
    free(ptr);   
}
