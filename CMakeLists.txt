cmake_minimum_required(VERSION 3.14)
project(kaentake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Build external dependencies
add_subdirectory(external)

# Build injector and laucher
add_subdirectory(src)