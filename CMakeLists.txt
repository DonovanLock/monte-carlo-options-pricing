cmake_minimum_required(VERSION 3.10)

# Project name and language
project(MonteCarlo LANGUAGES CXX)

# Specify the C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify source files
set(SOURCES
    src/main.cpp
    src/MonteCarlo.cpp
    src/Utils.cpp
)

# Add executable target
add_executable(MonteCarlo ${SOURCES})

# Add include directory so compiler finds your headers
target_include_directories(MonteCarlo PRIVATE include)

# Optional: Enable compiler warnings
if(MSVC)
    target_compile_options(MonteCarlo PRIVATE /W4 /permissive-)
else()
    target_compile_options(MonteCarlo PRIVATE -Wall -Wextra -Wpedantic)
endif()