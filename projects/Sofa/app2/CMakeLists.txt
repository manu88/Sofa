
cmake_minimum_required(VERSION 3.7.2)

project(app2 C)



add_executable(app2 src/main.c)

target_link_libraries(app2 sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:app2>")