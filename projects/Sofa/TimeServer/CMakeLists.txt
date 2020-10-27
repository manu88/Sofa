
cmake_minimum_required(VERSION 3.7.2)

project(TimeServer C)



add_executable(TimeServer src/main.c)

target_link_libraries(TimeServer sel4 sel4runtime muslc sel4muslcsys sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:TimeServer>")