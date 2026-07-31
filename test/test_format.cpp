#include <doctest.h>
#include <decoterm/format.hpp>

TEST_CASE("format: StyledFormat: asserts") {
    using namespace deco;
    StyledFormat sfmt;
}

TEST_CASE("format: StyledFormat: format_to()") {
    using namespace deco;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));
}

TEST_CASE("format: StyledFormat: format()") {

}
