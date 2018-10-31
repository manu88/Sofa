cmake_minimum_required(VERSION 3.7.2)

project(libdatastruct C)


file(GLOB deps src/*.c)
list(SORT deps)

add_library(libdatastruct ${deps})


target_include_directories(libdatastruct PUBLIC include) 

target_link_libraries(libdatastruct sel4muslcsys muslc)


add_compile_options(-Wall -Werror -W -Wextra)
