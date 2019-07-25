cmake_minimum_required(VERSION 3.7.2)

project(kernel_task C)


# list of apps to include in the cpio archive
get_property(cpio_apps GLOBAL PROPERTY apps_property)

MakeCPIO(archive.o "${cpio_apps}")

file(GLOB deps src/*.c src/*/*.c)
list(SORT deps)


add_executable(kernel_task ${deps} archive.o)

target_compile_definitions(kernel_task PRIVATE KERNEL_TASK=1)

target_link_libraries(kernel_task 
	sel4_autoconf
	sel4runtime
	muslc
	sel4muslcsys
	sel4allocman
	cpio
	libSysCall
)

DeclareRootserver(kernel_task)
GenerateSimulateScript()
