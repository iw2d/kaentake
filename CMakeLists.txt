cmake_minimum_required(VERSION 3.14)
project(kaentake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Build External Dependencies
add_subdirectory(external)

# Build Injector and Launcher
add_subdirectory(src)
