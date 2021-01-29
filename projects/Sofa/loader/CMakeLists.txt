
cmake_minimum_required(VERSION 3.7.2)

project(loader C)



add_executable(loader src/main.c)

target_link_libraries(loader sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:loader>")