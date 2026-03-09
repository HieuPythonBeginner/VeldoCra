# VeldoCra Compiler - CMakeLists.txt
# Project: VeldoCra (formerly TempestLang)
# Compiler: velc
# Language: C++23

cmake_minimum_required(VERSION 3.20)
project(VeldoCra LANGUAGES CXX)

# C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for maximum performance
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -mtune=native
        -ffast-math
        -funroll-loops
        -fno-exceptions
        -fno-rtti
    )
    # Enable inline assembly
    add_compile_options(-masm=intel)
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(DRIVER_SOURCES
    src/driver/driver.cpp
)

set(FRONTEND_SOURCES
    src/frontend/lexer/lexer.cpp
    src/frontend/parser/parser.cpp
    src/frontend/ast/ast.cpp
    src/frontend/source/source.cpp
)

set(MIDDLE_SOURCES
    src/middle/types/type.cpp
    src/middle/ownership/ownership.cpp
    src/middle/ir/ir.cpp
)

set(BACKEND_SOURCES
    src/backend/vm/vm.cpp
    src/backend/codegen/codegen.cpp
)

# Main executable
add_executable(velc
    src/main.cpp
    ${DRIVER_SOURCES}
    ${FRONTEND_SOURCES}
    ${MIDDLE_SOURCES}
    ${BACKEND_SOURCES}
)

# Target properties
target_compile_features(velc PRIVATE cxx_std_23)

# Installation
install(TARGETS velc RUNTIME DESTINATION bin)

