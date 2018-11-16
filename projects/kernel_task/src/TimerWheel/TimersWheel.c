/*
 * Copyright (c) 2016 FlyLab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
//
//  TimersWheel.c
//  GroundBase
//
//  Created by Manuel Deneu on 21/11/2016.
//  Copyright © 2016 FlyLab. All rights reserved.
//


#include "TimersWheel.h"
#include "timeout.h"
#include <stdlib.h>
#define TICK_TIME (timeout_t) 1


const TimerTick TimerTickInvalid = ~TIMEOUT_C(0);

int TimersWheelInit( TimersWheel* timerWheel)
{
    timerWheel->timers = timeouts_open( TICK_TIME,  NULL);
    return timerWheel->timers != NULL;
}

TimersWheel* TimersWheelAlloc(void)
{
    TimersWheel* self = malloc( sizeof( TimersWheel ));
    
    if( self && TimersWheelInit(self))
    {
        return self;
    }
    free(self);
    return NULL;
    
}

int TimersWheelDeInit( TimersWheel* timerWheel)
{
    timeouts_close( timerWheel->timers);
    
    return 1;
}

void TimerWheelRelease( TimersWheel* timerWheel)
{
    TimersWheelDeInit( timerWheel);
    free(timerWheel);
}


int TimerWheelAddTimer( TimersWheel* timerWheel , Timer * timer , TimerTick timePts )
{
    if( timerWheel == NULL)
        return 0;
    
    timeouts_add(timerWheel->timers, timer, timePts );
    
    return 1;
}

int TimerWheelRemoveTimer( TimersWheel* timerWheel , Timer * timer )
{
    if( timerWheel == NULL)
        return 0;
 
    timeouts_del(timerWheel->timers, timer);
    
    return 1;
}

TimerTick TimerWheelGetTimeout( const TimersWheel* timerWheel)
{
    return timeouts_timeout( timerWheel->timers );
}

int TimerWheelStep( TimersWheel* timerWheel , TimerTick step)
{
    if( timerWheel == NULL)
        return 0;
    
    timeouts_step( timerWheel->timers, step );
    return 1;
}

Timer* TimerWheelGetFiredTimers( TimersWheel* timerWheel)
{
    return timeouts_get( timerWheel->timers );
}

int TimerInit(Timer* timer , void* userData,  uint8_t oneShot)
{
    timer = timeout_init( timer ,  oneShot?TIMEOUT_ABS :  TIMEOUT_INT);
    
    timer->callback.arg = userData;
    
    return 1;
}

Timer* TimerAlloc( void* userData, uint8_t oneShot)
{
    struct timeout * timer = malloc(sizeof(struct timeout));
    
    if (timer && TimerInit(timer, userData, oneShot))
    {
        return timer;
    }
    

    return NULL;
}

void TimerRelease(Timer* timer)
{
    if( timer == NULL)
        return;
    
    timeout_del( timer );
    free(timer);
    
}

TimerTick TimerGetInterval( const Timer* timer)
{
    return timer->interval;
}
void* TimerGetUserContext( const Timer* timer)
{
    return timer->callback.arg;
}

