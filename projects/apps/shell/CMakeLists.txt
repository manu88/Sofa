cmake_minimum_required(VERSION 3.7.2)

project(shell C) 

add_executable(shell src/main.c)


target_link_libraries(shell muslc libSysCall)

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:shell>")
