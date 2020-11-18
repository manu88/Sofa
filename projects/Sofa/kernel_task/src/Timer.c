#include <platsupport/irq.h>
#include <vka/capops.h>
#include <platsupport/local_time_manager.h>
#include <sel4platsupport/device.h>
#include "Timer.h"
#include "Environ.h"


static ps_irq_register_fn_t irq_register_fn_copy;


static irq_id_t sel4test_timer_irq_register(UNUSED void *cookie, ps_irq_t irq, irq_callback_fn_t callback,
                                            void *callback_data)
{
    KernelTaskContext* env = getKernelTaskContext();
    static int num_timer_irqs = 0;

    int error;

    ZF_LOGF_IF(!callback, "Passed in a NULL callback");

    ZF_LOGF_IF(num_timer_irqs >= MAX_TIMER_IRQS, "Trying to register too many timer IRQs");

    /* Allocate the IRQ */
    error = sel4platsupport_copy_irq_cap(&env->vka, &env->simple, &irq,
                                         &env->timer_irqs[num_timer_irqs].handler_path);
    ZF_LOGF_IF(error, "Failed to allocate IRQ handler");

    /* Allocate the root notifitcation if we haven't already done so */
    if (env->timer_notification.cptr == seL4_CapNull) {
        error = vka_alloc_notification(&env->vka, &env->timer_notification);
        ZF_LOGF_IF(error, "Failed to allocate notification object");
    }

    /* Mint a notification for the IRQ handler to pair with */
    error = vka_cspace_alloc_path(&env->vka, &env->badged_timer_notifications[num_timer_irqs]);
    ZF_LOGF_IF(error, "Failed to allocate path for the badged notification");
    cspacepath_t root_notification_path = {0};
    vka_cspace_make_path(&env->vka, env->timer_notification.cptr, &root_notification_path);
    seL4_Word badge = TIMER_BADGE; // BIT(num_timer_irqs)
    printf("Mint timer with value %lu\n", BIT(num_timer_irqs));
    error = vka_cnode_mint(&env->badged_timer_notifications[num_timer_irqs], &root_notification_path,
                           seL4_AllRights, badge);
    ZF_LOGF_IF(error, "Failed to mint notification for timer");

    /* Pair the notification and the handler */
    error = seL4_IRQHandler_SetNotification(env->timer_irqs[num_timer_irqs].handler_path.capPtr,
                                            env->badged_timer_notifications[num_timer_irqs].capPtr);
    ZF_LOGF_IF(error, "Failed to pair the notification and handler together");

    /* Ack the handler so interrupts can come in */
    error = seL4_IRQHandler_Ack(env->timer_irqs[num_timer_irqs].handler_path.capPtr);
    ZF_LOGF_IF(error, "Failed to ack the IRQ handler");

    /* Fill out information about the callbacks */
    env->timer_cbs[num_timer_irqs].callback = callback;
    env->timer_cbs[num_timer_irqs].callback_data = callback_data;

    return num_timer_irqs++;
}


static void init_timer(void)
{
    if (config_set(CONFIG_HAVE_TIMER)) 
    {
        KernelTaskContext* env = getKernelTaskContext();
        int error;

        /* setup the timers and have our wrapper around simple capture the IRQ caps */
        error = ltimer_default_init(&env->ltimer, env->ops, NULL, NULL);
        ZF_LOGF_IF(error, "Failed to setup the timers");

        error = vka_alloc_notification(&env->vka, &env->timer_notify_test);
        ZF_LOGF_IF(error, "Failed to allocate notification object for tests");

        error = seL4_TCB_BindNotification(simple_get_tcb(&env->simple), env->timer_notification.cptr);
        ZF_LOGF_IF(error, "Failed to bind timer notification to sel4test-driver\n");

        /* set up the timer manager */
        tm_init(&env->tm, &env->ltimer, &env->ops, TIMER_ID_COUT);
    }
}

int TimerInit()
{
    KernelTaskContext* env = getKernelTaskContext();

    irq_register_fn_copy = env->ops.irq_ops.irq_register_fn;
    env->ops.irq_ops.irq_register_fn = sel4test_timer_irq_register;

    init_timer();
    /* Restore the IRQ interface's register function */
    env->ops.irq_ops.irq_register_fn = irq_register_fn_copy;

    return 0;

}