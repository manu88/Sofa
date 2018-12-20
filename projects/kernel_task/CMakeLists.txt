cmake_minimum_required(VERSION 3.7.2)



project(kernel_task C)

# Copy CPIOFiles 
file(COPY CPIOFiles DESTINATION ".")

# add file to cpio : FIXME : do it for each file in CPIOFiles
#set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE_DIR:kernel_task>/CPIOFiles/hello.txt")
#set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE_DIR:init>/CPIOFiles/readme.txt")


# list of apps to include in the cpio archive
get_property(cpio_apps GLOBAL PROPERTY apps_property)

MakeCPIO(archive.o "${cpio_apps}")

file(GLOB deps src/*.c src/*/*.c)
list(SORT deps)


set(LibPicotcp ON CACHE BOOL "" FORCE)
config_set(LIB_PICOTCP LIB_PICOTCP True)

add_executable(kernel_task ${deps} archive.o)

target_link_libraries(kernel_task sel4muslcsys sel4allocman sel4platsupport sel4 muslc cpio sel4utils  libdatastruct picotcp ethdrivers) 

# warn about everything
add_compile_options(-Wall -Werror -W -Wextra)

# Set this image as the rootserver
DeclareRootserver(kernel_task)

if(CONFIG_BENCHMARK_TRACK_UTILISATION)
	message("BENCHMARK_TRACK_UTILISATION is set")
else()
	message("BENCHMARK_TRACK_UTILISATION is NOT set")
endif()
