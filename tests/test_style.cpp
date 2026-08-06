#include <decoterm/style.hpp>
#include <doctest.h>

#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

// NOLINTBEGIN

namespace {

auto esc(std::string_view params) -> std::string {
    std::string result = "\x1b[";
    result += params;
    result += 'm';
    return result;
}

auto sgr_params(deco::Style style) -> std::string {
    std::string result;
    style.to_sgr_params(std::back_inserter(result));
    return result;
}

struct Value {
    int value = 128;
};

auto operator<<(std::ostream& os, Value value) -> std::ostream& {   //NOLINT
    os << value.value;
    return os;
}

} // namespace

TEST_CASE("style: Style::to_sgr_params") {
    using namespace deco;

    CHECK(sgr_params(Style()) == "");
    CHECK(sgr_params(bold) == "1");
    CHECK(sgr_params(bold | italic | underline) == "1;3;4");
    CHECK(sgr_params(fg(red) | bg(blue)) == "31;44");
    CHECK(sgr_params(fg(rgb(1, 2, 3)) | bg(rgb(4, 5, 6))) ==
          "38;2;1;2;3;48;2;4;5;6");
    CHECK(sgr_params(bold | fg(red) | bg(blue) | underline) == "31;44;1;4");
}

TEST_CASE("style: Style composition") {
    using namespace deco;

    const Style composed = fg(red) | bg(blue) | bold | fg(green) | italic;

    CHECK(composed.emphasis() == (Style::bold | Style::italic));
    CHECK(composed.fg() == green);
    CHECK(composed.bg() == blue);
    CHECK(sgr_params(composed) == "32;44;1;3");

    const Style rhs_without_colors = fg(red) | bg(blue) | bold;
    CHECK((rhs_without_colors | italic).fg() == red);
    CHECK((rhs_without_colors | italic).bg() == blue);
    CHECK(sgr_params(rhs_without_colors | italic) == "31;44;1;3");
}

TEST_CASE("style: Style::to_escape") {
    using namespace deco;
    CHECK((fg(red) | bg(blue) | bold).to_escape() == esc("31;44;1"));
}

TEST_CASE("style: null style") {
    using namespace deco;
    CHECK(null_style.is_null());
    CHECK(null_style.to_escape() == "");
}

TEST_CASE("style: AbsoluteStyle::to_escape") {
    using namespace deco;

    // Shouldn't output ';' when inner Style is null.
    CHECK(abs(null_style).to_escape() == "\x1b[m");
    CHECK(abs(bold).to_escape() == "\x1b[;1m");
}

TEST_CASE("style: StyledRef") {
    using namespace deco;

    std::ostringstream os;
    Value value {64};

    os << styled(Value {32}, bold);
    os << styled(value, italic);

    CHECK(os.str()
          == bold.to_escape() + std::string("32")
                 + abs(null_style).to_escape()
                 + italic.to_escape() + std::string("64")
                 + abs(null_style).to_escape());
}

// NOLINTEND
