cmake_minimum_required(VERSION 3.7.2)

project(kernel_task C)


# Copy CPIOFiles 
file(COPY CPIOFiles DESTINATION ".")
set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE_DIR:kernel_task>/CPIOFiles/file")

include(../../util_libs/liblwip/lwip_helpers.cmake)
set(LibLwip ON CACHE BOOL "" FORCE)
AddLWIPConfiguration(../../util_libs/liblwip/default_opts)

#list of apps to include in the cpio archive
get_property(cpio_apps GLOBAL PROPERTY apps_property)
include(cpio)
MakeCPIO(archive.o "${cpio_apps}")

file(GLOB deps src/*.c src/*/*.c src/*/*/*.c)
list(SORT deps)

add_executable(kernel_task ${deps} archive.o)

target_include_directories(kernel_task PRIVATE src)
target_include_directories(kernel_task PRIVATE src/DeviceKit src/Drivers src/FS)

# X86 include specifics
target_include_directories(kernel_task PRIVATE src/Drivers/X86)

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
        lwip
        ethdrivers
        sel4sync
)

# Set this image as the rootserver
include(rootserver)
DeclareRootserver(kernel_task)