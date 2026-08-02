set(EXAMPLE_SRCS
    quickstart.cpp
)

foreach(source IN LISTS EXAMPLE_SRCS)
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE decoterm)
    target_compile_features(${target} PRIVATE cxx_std_23)
endforeach()

