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
#include <sel4utils/thread.h>
#include "Thread.h"
#include <sync/recursive_mutex.h>

typedef struct _KThread KThread;

typedef int (*KThreadMain)(KThread* thread, void *arg);

typedef struct _KThread
{
    ThreadBase _base; // needs to remain fisrt!!
    seL4_CPtr ep;
    sel4utils_thread_t native;
    KThreadMain mainFunction;
    const char* name; // can be NULL

}KThread;

void KThreadInit(KThread* t);

int KThreadRun(KThread* t, int prio, void* arg);
void KThreadCleanup(KThread* t);


int KThreadSleep(KThread* t, int ms);
void KThreadExit(KThread* t, int code);

// Works only in threads, so be sure not to call it in main Thread!
int KSleep(int ms);


typedef sync_recursive_mutex_t KMutex;


int KMutexNew(KMutex* mutex);
int KMutexDelete(KMutex* mutex);

int KMutexLock(KMutex* mutex);
int KMutexUnlock(KMutex* mutex);