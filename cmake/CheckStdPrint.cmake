include_guard(GLOBAL)

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

function(decoterm_check_std_print result_var)
    cmake_push_check_state(RESET)

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    check_cxx_source_compiles([=[
    #include <print>
    int main() { std::print("{}",42); return 0; }
]=] _decoterm_has_std_print)

    cmake_pop_check_state()

    set(${result_var} "${_decoterm_has_std_print}" PARENT_SCOPE)

endfunction()
