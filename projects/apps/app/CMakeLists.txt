cmake_minimum_required(VERSION 3.7.2)

project(app C) # create a new C project called 'Hello' 


add_executable(app src/main.c) # add files to our project. Paths are relative to this file.

set_property(TARGET app APPEND_STRING PROPERTY LINK_FLAGS " -u __vsyscall_ptr ")

target_link_libraries(app   muslc libSysCall)
#sel4muslcsys sel4

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:app>")



