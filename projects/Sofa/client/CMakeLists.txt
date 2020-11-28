
cmake_minimum_required(VERSION 3.7.2)

project(client C)



add_executable(client src/main.c)

target_link_libraries(client sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:client>")