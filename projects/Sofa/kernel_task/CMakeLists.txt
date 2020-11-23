cmake_minimum_required(VERSION 3.7.2)

project(kernel_task C) # create a new C project called 'init' 


#list of apps to include in the cpio archive
get_property(cpio_apps GLOBAL PROPERTY apps_property)
include(cpio)
MakeCPIO(archive.o "${cpio_apps}")

file(GLOB deps src/*.c src/*/*.c)
list(SORT deps)

add_executable(kernel_task ${deps} archive.o)



target_link_libraries(
    kernel_task
    PUBLIC
        muslc
        sel4platsupport
        sel4allocman
        sel4runtime
        sel4muslcsys
        cpio
        sel4vka
        sofa
        utils
        libACPI
        pci
)

# Set this image as the rootserver
include(rootserver)
DeclareRootserver(kernel_task)