# unit test
add_executable(decoterm_test
    main.cpp
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
