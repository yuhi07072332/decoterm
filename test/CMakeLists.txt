# unit test
add_executable(decoterm_test
    test_color.cpp
    test_style.cpp
    test_output.cpp
    test_format.cpp
)

target_include_directories(decoterm_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(decoterm_test
    PRIVATE
    decoterm
)

target_compile_features(decoterm_test PRIVATE cxx_std_23)

add_test(NAME decoterm_test COMMAND decoterm_test)

# output test

set(OUTPUT_TEST_NAMES
    output
)

foreach(test_name IN LISTS OUTPUT_TEST_NAMES)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE decoterm)
    target_compile_features(${test_name} PRIVATE cxx_std_23)
endforeach()
