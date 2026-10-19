# C++20 tests
add_library(decoterm_test_cpp20 OBJECT
    test_style.cpp
    test_output.cpp
    test_sync.cpp
)

target_link_libraries(decoterm_test_cpp20 PRIVATE
    decoterm
    decoterm_dev_options
)

set_target_properties(decoterm_test_cpp20 PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(decoterm_test_cpp20 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# C++23 tests
add_executable(decoterm_test
    main.cpp
    test_format.cpp
)

target_link_libraries(decoterm_test PRIVATE
    decoterm
    decoterm_dev_options
    decoterm_test_cpp20
)

set_target_properties(decoterm_test PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(decoterm_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

add_test(NAME decoterm_test COMMAND decoterm_test)
