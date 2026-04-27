# Veldanava Compiler - CMakeLists.txt
# Project: Veldanava
# Compiler: veldanc
# Language: C++23

cmake_minimum_required(VERSION 3.20)
project(Veldanava LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O3 -march=native -mtune=native)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BUILD_STATIC "Build statically linked executable" OFF)
if(BUILD_STATIC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

set(DRIVER_SOURCES src/driver/driver.cpp)
set(FRONTEND_SOURCES src/frontend/lexer/lexer.cpp src/frontend/parser/parser.cpp src/frontend/ast/ast.cpp src/frontend/source/source.cpp)
set(MIDDLE_SOURCES src/middle/types/type.cpp src/middle/ownership/ownership.cpp src/middle/ir/ir.cpp)
set(STDLIB_SOURCES src/stdlib/stdlib.cpp)
set(BACKEND_SOURCES
    src/backend/vm/vm.cpp
    src/backend/vm/gc.cpp
    src/backend/codegen/codegen_fixed.cpp
    src/backend/native_codegen/opcode.cpp
    src/backend/native_codegen/assembler.cpp
    src/backend/native_codegen/elf.cpp
    src/backend/native_codegen/native_codegen.cpp
)

add_executable(veldanc
    src/main.cpp
    ${DRIVER_SOURCES}
    ${FRONTEND_SOURCES}
    ${MIDDLE_SOURCES}
    ${STDLIB_SOURCES}
    ${BACKEND_SOURCES}
)

target_compile_features(veldanc PRIVATE cxx_std_23)

install(TARGETS veldanc RUNTIME DESTINATION bin)