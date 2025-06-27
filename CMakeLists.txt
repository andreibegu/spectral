cmake_minimum_required(VERSION 3.20.0)
project(PallasSpec2IR)

# 17 required by antlr4
SET (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

###########################################################################################
#   Antlr4 build based on https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake   #
###########################################################################################

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# Specify the version of the antlr4 library needed for this project.
set(ANTLR4_TAG 4.13.1)

# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/res/dependencies/antlr-4.13.1-complete.jar)

# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

#############################################################################################
#   LLVM include based on https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project   #
#############################################################################################

find_package(LLVM 17 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find the libraries that correspond to the LLVM components that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader linker passes analysis)

# Enable warnings
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

message("Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

# Add the subdirectories
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)
add_subdirectory(res)

# Required to fix import errors in VSCode
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)