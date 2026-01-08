cmake_minimum_required(VERSION 3.20)

project(gcdtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(gcdtest
    src/main.cpp
)

target_include_directories(gcdtest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(gcdtest PRIVATE -Wall -Wextra -Wpedantic)
