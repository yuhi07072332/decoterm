#unit test

include(FetchContent)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest
    GIT_TAG v2.5.2
)
FetchContent_MakeAvailable(doctest)

include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")

add_executable(decoterm_test
    test_color_esc.cpp
    test_style_esc.cpp
)

target_link_libraries(decoterm_test
    PRIVATE
    decoterm
    doctest::doctest
)

target_compile_features(decoterm_test PRIVATE cxx_std_20)

add_test(NAME decoterm_test COMMAND decoterm_test)

# output test

set(OUTPUT_TEST_NAMES
    test_style
    test_color
    test_color_support
    benchmark
)

foreach(test_name IN LISTS OUTPUT_TEST_NAMES)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE decoterm)
    target_compile_features(${test_name} PRIVATE cxx_std_23)
endforeach()
