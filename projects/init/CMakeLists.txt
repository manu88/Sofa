cmake_minimum_required(VERSION 3.7.2)



project(init C) # create a new C project called 'init' 


# list of apps to include in the cpio archive
get_property(cpio_apps GLOBAL PROPERTY apps_property)
MakeCPIO(archive.o "${cpio_apps}")

add_executable(init src/main.c src/ProcessTable.c src/Process.c src/Bootstrap.c src/Timer.c src/TimerWheel/timeout-bitops.c src/TimerWheel/timeout.c src/TimerWheel/TimersWheel.c archive.o)

target_link_libraries(init sel4muslcsys sel4allocman sel4platsupport sel4 muslc cpio sel4utils libSysCall ) 


# Set this image as the rootserver
DeclareRootserver(init)

