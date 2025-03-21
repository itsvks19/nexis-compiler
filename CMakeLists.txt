cmake_minimum_required(VERSION 3.22)
project(NexisCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Include directories
include_directories(include)

# Compiler executable
add_executable(nexis_compiler ${SOURCES})
