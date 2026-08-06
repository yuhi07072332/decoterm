cmake_minimum_required(VERSION 3.30)

option(DECOTERM_BUILD_TESTS "build tests" OFF)
option(DECOTERM_BUILD_EXAMPLES "build examples" OFF)
option(DECOTERM_BUILD_BENCHMARKS "build benchmarks" OFF)

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

#check <print> support
if(DECOTERM_BUILD_TESTS OR DECOTERM_BUILD_EXAMPLES OR DECOTERM_BUILD_BENCHMARKS)
    include(cmake/CheckStdPrint.cmake)
    decoterm_check_std_print(DECOTERM_HAS_STD_PRINT)
    if (NOT DECOTERM_HAS_STD_PRINT)
        message(FATAL_ERROR "Building decoterm tests / examples / benchmarks requires std::print, but your compiler doesn't support!")
    endif()
endif()

if(DECOTERM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(DECOTERM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(DECOTERM_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()
