# barretenberg
# copyright 2019 Spilsbury Holdings Ltd

cmake_minimum_required(VERSION 3.16)

include(cmake/toolchain.cmake)

set(PROJECT_VERSION 0.1.0)
project(Barretenberg
    DESCRIPTION "BN254 elliptic curve library, and PLONK SNARK prover"
    LANGUAGES CXX C)

option(DISABLE_ASM "Disable custom assembly" OFF)
option(DISABLE_ADX "Disable ADX assembly variant" OFF)
option(MULTITHREADING "Enable multi-threading" ON)
option(TESTING "Build tests" OFF)
option(BENCHMARKS "Build benchmarks" OFF)
option(PROFILING "Build profiling" ON)
option(APPLE_M1 "Compile for Apple M1 CPUs" OFF)

SET(GCC_COVERAGE_COMPILE_FLAGS "-Wunused-but-set-variable")

if(APPLE_M1)
    set(MULTITHREADING OFF)
    set(DISABLE_ASM ON)
    set(DISABLE_ADX ON)
    add_definitions(-DNDEBUG)
endif()

if(ARM)
    message(STATUS "Compiling for ARM.")
    set(DISABLE_ASM ON)
    set(RUN_HAVE_STD_REGEX 0)
    set(RUN_HAVE_POSIX_REGEX 0)
endif()

if(WASM)
    message(STATUS "Compiling for WebAssembly.")
    set(DISABLE_ASM ON)
    set(MULTITHREADING OFF)
    set(BENCHMARKS OFF)
    set(PROFILING OFF)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS ON)

include(GNUInstallDirs)
include(cmake/arch.cmake)
include(cmake/threading.cmake)
# include(cmake/gtest.cmake)
# include(cmake/benchmark.cmake)
include(cmake/module.cmake)

add_subdirectory(src/aztec)