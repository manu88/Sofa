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
#pragma once
#include <utils/circular_buffer.h>


#define MAKE_CIRC_SIZE(size) sizeof(circ_buf_t) + (size) -1  // '-1' 'cause 1 byte is already present in circ_buf_t

typedef char CircularBuffer;

static inline int CircularBufferInit(CircularBuffer* buffer, size_t size)
{
    return circ_buf_init(size, ((circ_buf_t *) buffer));
}

static inline size_t CircularBufferGetAvailableChar(const CircularBuffer* buffer)
{
    return ((circ_buf_t *) buffer)->tail - ((circ_buf_t *) buffer)->head;
}

static inline void CircularBufferPut(CircularBuffer* buffer, char data)
{
    circ_buf_put((circ_buf_t *) buffer, data);
}

static inline bool CircularBufferIsEmpty(const CircularBuffer* buffer)
{
    return circ_buf_is_empty((circ_buf_t *) buffer);
}

static inline char CircularBufferGet(CircularBuffer* buffer)
{
    return circ_buf_get((circ_buf_t *) buffer);
}


static inline void circ_buf_rem(circ_buf_t *cb)
{
    cb->tail = (cb->tail - 1) % cb->size;
}

static inline void CircularBufferRemove(CircularBuffer* buffer)
{
    circ_buf_rem((circ_buf_t *) buffer);
}