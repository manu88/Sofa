#pragma once

#include <sel4utils/thread.h>

typedef struct
{
    sel4utils_thread_t _thread;
}pthread_t;


typedef struct
{

}pthread_attr_t;

int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void*), void *restrict arg);
