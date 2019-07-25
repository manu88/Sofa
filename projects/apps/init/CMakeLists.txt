cmake_minimum_required(VERSION 3.7.2)

project(init C) 

add_executable(init src/main.c)

#set_property(TARGET init APPEND_STRING PROPERTY LINK_FLAGS " -u __vsyscall_ptr ")



target_link_libraries(init sel4_autoconf muslc sel4  sel4rt libSysCall )

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:init>")



