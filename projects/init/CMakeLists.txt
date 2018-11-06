cmake_minimum_required(VERSION 3.7.2)



project(init C) # create a new C project called 'init' 


# Copy CPIOFiles 
file(COPY CPIOFiles DESTINATION ".")

# add file to cpio : FIXME : do it for each file in CPIOFiles
set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE_DIR:init>/CPIOFiles/hello.txt")


# list of apps to include in the cpio archive
get_property(cpio_apps GLOBAL PROPERTY apps_property)

MakeCPIO(archive.o "${cpio_apps}")

file(GLOB deps src/*.c src/*/*.c)
list(SORT deps)

add_executable(init ${deps} archive.o)

target_link_libraries(init sel4muslcsys sel4allocman sel4platsupport sel4 muslc cpio sel4utils libSysCall libdatastruct) 

# warn about everything
add_compile_options(-Wall -Werror -W -Wextra)

# Set this image as the rootserver
DeclareRootserver(init)

