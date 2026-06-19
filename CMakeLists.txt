cmake_minimum_required(VERSION 3.30)

option(DECOTERM_BUILD_TEST "enable test" off)

project(decoterm
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(decoterm INTERFACE include/decoterm/decoterm.hpp)

target_include_directories(decoterm INTERFACE include)
target_compile_features(decoterm INTERFACE cxx_std_20)

if(DECOTERM_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

