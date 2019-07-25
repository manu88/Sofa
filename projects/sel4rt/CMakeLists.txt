#
# Copyright 2019, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(DATA61_BSD)
#
# Compile CRT for given arch and get crt0.o, crti.o, and crtn.o.
# Add crt0.o and crti.o to the start of all targets link flags.
# Add crtn.o to the end of all targets link flags.

# Compile sel4runtime library from src.
# Use src as private includes for crt and sel4runtime.
# Use include as public includes for crt and sel4runtime.

cmake_minimum_required(VERSION 3.12.0)

project(sel4rt C)

set(configure_string "")

list(
    APPEND
        sources
        src/crt1.c
        src/start.c
        src/start_root.c
        src/env.c
        src/init.c
        src/memset.c
        src/memcpy.c
)
if(("${KernelSel4Arch}" STREQUAL "aarch32") OR ("${KernelSel4Arch}" STREQUAL "arm_hyp"))
    list(
        APPEND
            sources src/sel4_arch/${KernelSel4Arch}/__aeabi_read_tp.s
            src/sel4_arch/${KernelSel4Arch}/__aeabi_read_tp_c.c
    )
endif()

# The sel4runtime library.
add_library(sel4rt STATIC ${crt_files} ${sources})
target_include_directories(
    sel4rt
    PRIVATE
        src
        src/mode/${KernelWordSize}
        src/arch/${KernelArch}
        src/sel4_arch/${KernelSel4Arch}
    PUBLIC
        include
        include/mode/${KernelWordSize}
        include/arch/${KernelArch}
        include/sel4_arch/${KernelSel4Arch}
)
add_dependencies(sel4rt sel4runtime_crt)

# A C library is still needed here to provide memcpy, stdint.h, and
# elf.h
target_link_libraries(sel4rt PUBLIC sel4 muslc sel4_autoconf libSysCall  PRIVATE sel4runtime_Config)
