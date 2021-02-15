
cmake_minimum_required(VERSION 3.7.2)

project(img C)



add_executable(img src/main.c src/Image.c)

target_link_libraries(img sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:img>")