
cmake_minimum_required(VERSION 3.7.2)

project(utests C)



add_executable(utests src/main.c)

target_link_libraries(utests sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:utests>")