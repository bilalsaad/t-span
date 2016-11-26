# Minimum version for CMake
cmake_minimum_required(VERSION 3.6)

# Project name
project(t-spanner)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR}")

# Source files
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cc")

# compiler options
# set for cpp14
set(CMAKE_CXX_STANDARD 14)

# compile rule
add_executable(t-spanner "${SOURCES}")

