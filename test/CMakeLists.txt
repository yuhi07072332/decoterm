include(FetchContent)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest
    GIT_TAG v2.5.2
)
FetchContent_MakeAvailable(doctest)

include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")

add_executable(decoterm_tests
    test_color_esc.cpp
    test_style_esc.cpp
)

target_link_libraries(decoterm_tests
    PRIVATE
    decoterm
    doctest::doctest
)

target_compile_features(decoterm_tests PRIVATE cxx_std_20)

add_test(NAME decoterm_tests COMMAND decoterm_tests)
