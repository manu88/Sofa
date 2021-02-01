
cmake_minimum_required(VERSION 3.7.2)

project(loader C)

file(GLOB deps src/*.c)
list(SORT deps)

add_library(loader STATIC ${deps})


target_include_directories(loader PUBLIC include) 

target_link_libraries(loader 
        sel4_autoconf
		sel4
        muslc
        sel4allocman
        sel4vka
)

# warn about everything
add_compile_options(-Wall -Werror -W -Wextra)