
cmake_minimum_required(VERSION 3.7.2)

project(initService C)



add_executable(initService src/main.c)

target_link_libraries(initService sel4 sel4runtime sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:initService>")