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
//  TimersWheel.h
//  GroundBase
//
//  Created by Manuel Deneu on 21/11/2016.
//  Copyright © 2016 FlyLab. All rights reserved.
//

#ifndef TimersWheel_h
#define TimersWheel_h

#include "timeout.h"

struct _TimersWheel
{
    struct timeouts* timers;
};

typedef struct _TimersWheel TimersWheel;
typedef struct timeout Timer;

typedef uint64_t TimerTick;

extern const TimerTick TimerTickInvalid;
//#define TimerTickInvalid (TimerTick ) ~UINT64_C(0)


/* **** **** **** **** **** **** **** **** **** **** */
/* TimersWheel interface */

TimersWheel* TimersWheelAlloc(void);
int TimersWheelInit( TimersWheel* timerWheel);
int TimersWheelDeInit( TimersWheel* timerWheel);
void TimerWheelRelease( TimersWheel* timerWheel);

int TimerWheelAddTimer( TimersWheel* timerWheel , Timer * timer , TimerTick timePts);
int TimerWheelRemoveTimer( TimersWheel* timerWheel , Timer * timer );

TimerTick TimerWheelGetTimeout( const TimersWheel* timerWheel);
int TimerWheelStep( TimersWheel* timerWheel , TimerTick step);

Timer* TimerWheelGetFiredTimers( TimersWheel* timerWheel);

/* **** **** **** **** **** **** **** **** **** **** */
/* Timer interface */

int    TimerInit(Timer* timer , void* userData,  uint8_t oneShot);
Timer* TimerAlloc(void* userData,  uint8_t oneShot);

void TimerRelease(Timer* timer);
TimerTick TimerGetInterval( const Timer* timer);

void* TimerGetUserContext( const Timer* timer);
#endif /* TimersWheel_h */
