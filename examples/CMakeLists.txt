set(EXAMPLE_SRCS
    quickstart-ostream.cpp
    quickstart-fmt.cpp
    colors.cpp
    styles.cpp
    terminal-info.cpp
)

foreach(source IN LISTS EXAMPLE_SRCS)
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE decoterm decoterm_dev_options)
    target_compile_features(${target} PRIVATE cxx_std_23)
endforeach()

