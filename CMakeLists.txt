cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

project(file_to_array C CXX ASM)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(file_to_array file_to_array.c)
