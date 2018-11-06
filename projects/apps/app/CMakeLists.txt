cmake_minimum_required(VERSION 3.7.2)

project(app C) 

add_executable(app src/main.c)

set_property(TARGET app APPEND_STRING PROPERTY LINK_FLAGS " -u __vsyscall_ptr ")

target_link_libraries(app   muslc libSysCall)

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:app>")



