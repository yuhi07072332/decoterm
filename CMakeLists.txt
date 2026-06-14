cmake_minimum_required(VERSION 3.15)

project(decoterm
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(decoterm INTERFACE
    include/decoterm/decoterm.hpp
)

target_include_directories(decoterm INTERFACE include)
target_compile_features(decoterm PRIVATE cxx_std_20)

