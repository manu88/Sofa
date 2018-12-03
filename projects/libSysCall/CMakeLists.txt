cmake_minimum_required(VERSION 3.7.2)

project(libSysCall C)

file(GLOB deps src/*.c)
list(SORT deps)

add_library(libSysCall EXCLUDE_FROM_ALL  ${deps})

target_include_directories(libSysCall PUBLIC include) 

target_link_libraries(libSysCall Configuration  sel4 sel4muslcsys)

# warn about everything
add_compile_options(-Wall -Werror -W -Wextra)
