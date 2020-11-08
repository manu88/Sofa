#pragma once

#include <sel4utils/thread.h>


typedef struct {
    void *(*start_routine)(void*);
    void *restrict arg;
}_ThreadArgs;

typedef struct
{
    sel4utils_thread_t _thread;
    vka_object_t local_endpoint;
    _ThreadArgs arg;
}pthread_t;


typedef struct
{

}pthread_attr_t;

int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void*), void *restrict arg);


int pthread_join(pthread_t thread, void **retval);