cmake_minimum_required(VERSION 3.7.2)

project(shell C) 


file(GLOB deps src/*.c )
list(SORT deps)

add_executable(shell ${deps} )


target_link_libraries(shell muslc libSysCall)

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:shell>")
