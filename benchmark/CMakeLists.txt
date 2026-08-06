set(BENCH_SRCS
    style.cpp
    styled_out.cpp
)

foreach(source IN LISTS BENCH_SRCS)
    get_filename_component(target ${source} NAME_WE)
    add_executable(bench_${target} ${source})
    target_link_libraries(bench_${target} PRIVATE decoterm)
    target_compile_features(bench_${target} PRIVATE cxx_std_23)
endforeach()

