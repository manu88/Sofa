
cmake_minimum_required(VERSION 3.7.2)

project(sofa C)

file(GLOB deps src/*.c)
list(SORT deps)

add_library(sofa STATIC ${deps})


target_include_directories(sofa PUBLIC include) 

target_link_libraries(sofa 
        sel4_autoconf
		sel4
        muslc
        sel4muslcsys
        sel4allocman
)

# warn about everything
add_compile_options(-Wall -Werror -W -Wextra)