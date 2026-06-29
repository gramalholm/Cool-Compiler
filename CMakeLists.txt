cmake_minimum_required(VERSION 3.10)

project(Cool-Compiler)

find_package(fmt CONFIG REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cool-compiler
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/semantic/semantic.cpp
    src/sym_table/sym_table.cpp
    src/codegen/codegen.cpp
    headers/lexer.h
    headers/parser.h
    headers/semantic.h
    headers/sym_table.h
    headers/codegen.h
)

target_include_directories(cool-compiler PRIVATE
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/codegen
)

target_link_libraries(cool-compiler PRIVATE fmt::fmt)
