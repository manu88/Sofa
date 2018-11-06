cmake_minimum_required(VERSION 3.7.2)

project(TestSysCalls C) 

add_executable(TestSysCalls src/main.c)


target_link_libraries(TestSysCalls muslc libSysCall)

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:TestSysCalls>")
