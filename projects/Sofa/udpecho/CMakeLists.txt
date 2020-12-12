
cmake_minimum_required(VERSION 3.7.2)

project(udpecho C)



add_executable(udpecho src/main.c)

target_link_libraries(udpecho sel4 sel4runtime muslc sofa)




set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:udpecho>")