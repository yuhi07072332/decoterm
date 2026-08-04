cmake_minimum_required(VERSION 3.30)

option(DECOTERM_BUILD_TEST "build test" OFF)
option(DECOTERM_BUILD_EXAMPLE "build example" OFF)
option(DECOTERM_BUILD_BENCHMARK "build benchmark" OFF)

project(decoterm
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(decoterm INTERFACE include/decoterm/decoterm.hpp)

target_include_directories(decoterm
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(decoterm INTERFACE cxx_std_20)

if(DECOTERM_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(DECOTERM_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

if (DECOTERM_BUILD_BENCHMARK)
    add_subdirectory(benchmark)
endif()
