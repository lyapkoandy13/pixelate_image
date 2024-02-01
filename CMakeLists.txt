cmake_minimum_required(VERSION 3.12)
project(pixelate_image VERSION 0.0.1 DESCRIPTION "Pixalates image to terminal" LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

add_library(c_setup INTERFACE)
target_compile_options(c_setup INTERFACE -Wall -Wpedantic -Wextra)
target_include_directories(c_setup INTERFACE ${PROJECT_SOURCE_DIR})

add_subdirectory(vendor)
add_subdirectory(${PROJECT_NAME})