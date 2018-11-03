//
//  TimersWheel_UnitTests.c
//  DevelSofaInit
//
//  Created by Manuel Deneu on 03/11/2018.
//  Copyright © 2018 Manuel Deneu. All rights reserved.
//

#include "TimersWheel_UnitTests.h"
#include <assert.h>
#include "TimersWheel.h"

int TimersWheel_UnitTests()
{
    TimersWheel wheel;
    assert(TimersWheelInit(&wheel));
    
    assert(TimerWheelGetFiredTimers(&wheel) == NULL);
    
    Timer t1;
    TimerInit(&t1, &wheel, 0);
    
    Timer t2;
    TimerInit(&t2, &wheel, 0);
    
    TimerWheelAddTimer(&wheel, &t1, 1000);
    TimerWheelAddTimer(&wheel, &t2, 4000);
    
    assert(TimerWheelGetFiredTimers(&wheel) == NULL);
    
    
    
    
    int accum = 0;
    //while (1)
    for(int i = 0;i<10;i++)
    {
        TimerTick remain = TimerWheelGetTimeout(&wheel);
        
        printf("Timeout %llu\n" , remain);
        
        TimerWheelStep(&wheel, remain);
        
        accum++;
        
        Timer* f = TimerWheelGetFiredTimers(&wheel);
        
        if(f)
        {
            assert(f == &t1 || f == &t2);
            printf("Got fired at %i\n" , accum);
        }
        
        
    }
    
    return 1;
}
