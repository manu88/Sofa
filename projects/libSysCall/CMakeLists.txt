cmake_minimum_required(VERSION 3.7.2)

project(libSysCall C)

add_library(libSysCall EXCLUDE_FROM_ALL  src/SysClient.c)
target_include_directories(libSysCall PUBLIC include) 


target_link_libraries(libSysCall Configuration  sel4 sel4muslcsys)
