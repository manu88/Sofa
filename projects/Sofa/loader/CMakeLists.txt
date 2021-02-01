
cmake_minimum_required(VERSION 3.7.2)

project(test_loader C)



add_executable(test_loader src/main.c)

target_link_libraries(test_loader sel4 sel4runtime muslc sofa loader)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:test_loader>")