cmake_minimum_required(VERSION 3.10)

project(Cool-Compiler)

find_package(fmt CONFIG REQUIRED)

add_executable(cool-compiler
    src/main.cpp
)

target_include_directories(cool-compiler PRIVATE src)

target_link_libraries(cool-compiler PRIVATE fmt::fmt)
