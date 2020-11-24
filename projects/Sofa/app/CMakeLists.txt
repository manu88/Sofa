
cmake_minimum_required(VERSION 3.7.2)

project(app C)



add_executable(app src/main.c)

target_link_libraries(app sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:app>")