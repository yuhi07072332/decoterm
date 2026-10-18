add_library(decoterm_dev_options INTERFACE)

if(MSVC)
    target_compile_definitions(decoterm_dev_options INTERFACE
        _CRT_SECURE_NO_WARNINGS
    )
endif()

target_compile_options(decoterm_dev_options INTERFACE
    "$<$<CXX_COMPILER_FRONTEND_VARIANT:GNU>:-Wall;-Wextra;-Wpedantic>"
    "$<$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>:/W4;/permissive->"
)
