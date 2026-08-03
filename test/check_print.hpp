#include <version>

#if !(defined(__cpp_lib_print) && __cpp_lib_print >= 202403L)
#ifdef DECO_REQUIRE_STD_PRINT
#error "This example uses <print>, but your compiler doesn't support!"
#endif
#else 
#define DECO_HAS_STD_PRINT
#include <print>
#endif
