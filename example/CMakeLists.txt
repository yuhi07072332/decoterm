
file(GLOB EXAMPLE_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

foreach(source IN LISTS EXAMPLE_SRCS)
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE decoterm)
    target_compile_features(${target} PRIVATE cxx_std_20)
endforeach()

