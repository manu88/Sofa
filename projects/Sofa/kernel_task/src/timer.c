
#include <platsupport/local_time_manager.h> // tm_init
#include "timer.h"
#include "env.h"

void Timer_init(Environ* env)
{
    int error;

    /* setup the timers and have our wrapper around simple capture the IRQ caps */
    error = ltimer_default_init(&env->ltimer, env->ops, NULL, NULL);
    ZF_LOGF_IF(error, "Failed to setup the timers");

    /* Allocate the root notification if we haven't already done so */
    error = vka_alloc_notification(&env->vka, &env->timer_notification);
    ZF_LOGF_IF(error, "Failed to allocate notification object");


    error = seL4_TCB_BindNotification(simple_get_tcb(&env->simple), env->timer_notification.cptr);
    ZF_LOGF_IF(error, "Failed to bind timer notification to sel4test-driver\n");

    /* set up the timer manager */
    error = tm_init(&env->tm, &env->ltimer, &env->ops, 1);
    ZF_LOGF_IF(error, "Failed to create Time manager\n");
}