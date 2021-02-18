
cmake_minimum_required(VERSION 3.7.2)

project(doom C)

file(GLOB deps src/*.c)
list(SORT deps)

add_executable(doom ${deps})

target_link_libraries(doom sel4 sel4runtime muslc sofa)

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:doom>")