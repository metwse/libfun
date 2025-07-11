cmake_minimum_required(VERSION 3.28)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

project(libfun VERSION 0.0.0)

configure_file(include/meta.h.in include/meta.h)

target_include_directories(Tests PUBLIC
                           "${PROJECT_BINARY_DIR}")

add_executable(Tests tests/main.c)
