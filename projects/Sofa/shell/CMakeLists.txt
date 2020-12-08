
cmake_minimum_required(VERSION 3.7.2)

project(shell C)



add_executable(shell src/main.c src/files.c)

target_link_libraries(shell sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:shell>")