/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */
#include <sel4runtime/start.h>
#include <stddef.h>

#pragma once

/*
 * This performs all of the work of loading the execution environment.
 * It mainly operates be loading values out of the environment variables
 * and auxiliary vectors and into variables accessible via interface
 * functions.
 */
void __sel4runtime_load_env(
    char const *arg0,
    char const * const *envp,
    auxv_t const auxv[]
);
