cmake_minimum_required(VERSION 3.12)

add_executable(MainExec main.c)
target_link_libraries(MainExec c_setup stb::stb)
