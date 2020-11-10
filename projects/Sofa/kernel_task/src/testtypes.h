#pragma once

void basic_set_up(uintptr_t e);

seL4_SlotRegion copy_untypeds_to_process(sel4utils_process_t *process, vka_object_t *untypeds, int num_untypeds,
                                                driver_env_t env);