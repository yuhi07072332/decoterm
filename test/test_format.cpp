#include <doctest.h>
#include <decoterm/format.hpp>

TEST_CASE("format: StyledFormat: asserts") {
    using namespace deco;
    StyledFormat sfmt;
    // 是不是不太能检测static assertion啊，如果不行就不实现了
}

TEST_CASE("format: StyledFormat: format_to()") {
    using namespace deco;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));
}

TEST_CASE("format: StyledFormat: format_to() with style") {
    using namespace deco;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));
}

TEST_CASE("format: StyledFormat: print()") {

}

TEST_CASE("format: StyledFormat: print() with style") {

}

TEST_CASE("format: StyledFormat: set stream()") {

}

TEST_CASE("format: StyledFormat: style nesting") {

}
