set(EXAMPLE_CPP20_SRCS
    quickstart-ostream.cpp
)

set(EXAMPLE_CPP23_SRCS
    colors.cpp
    quickstart-fmt.cpp
    styles.cpp
    terminal-info.cpp
) 

foreach(source IN LISTS EXAMPLE_CPP20_SRCS)
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE decoterm decoterm_dev_options)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endforeach()

foreach(source IN LISTS EXAMPLE_CPP23_SRCS)
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE decoterm decoterm_dev_options)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endforeach()

