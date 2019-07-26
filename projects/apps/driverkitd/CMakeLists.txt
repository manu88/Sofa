cmake_minimum_required(VERSION 3.7.2)

project(driverkitd C) 

add_executable(driverkitd src/main.c)

#set_property(TARGET init APPEND_STRING PROPERTY LINK_FLAGS " -u __vsyscall_ptr ")



target_link_libraries(driverkitd sel4_autoconf muslc sel4  sel4rt libSysCall libACPI )

set_property(GLOBAL APPEND PROPERTY apps_property "$<TARGET_FILE:driverkitd>")



