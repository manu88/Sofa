cmake_minimum_required(VERSION 3.7.2)

project(kernel_task C) # create a new C project called 'init' 




#list of apps to include in the cpio archive
get_property(cpio_apps GLOBAL PROPERTY apps_property)
include(cpio)
MakeCPIO(archive.o "${cpio_apps}")


add_executable(kernel_task src/main.c src/env.c src/timer.c src/utils.c archive.o)

target_link_libraries(
    kernel_task
    PUBLIC
        muslc
        sel4allocman
        sel4runtime
        sel4muslcsys
        cpio
        sel4vka
        sofa
)

# Set this image as the rootserver
include(rootserver)
DeclareRootserver(kernel_task)