
#include <decoterm/format.hpp>

#include "unit_test.hpp"
#include <doctest.h>

#include <format>
#include <iterator>
#include <sstream>
#include <string>
#include <version>

// NOLINTBEGIN


using namespace deco;
using doctest::test_suite;

TEST_CASE("formatter" * test_suite("formatter")) {
    std::string result;
    std::ostringstream expected;

    SUBCASE("Style") {
        result = std::format("{}text", bold | fg(yellow));
        expected << (bold | fg(yellow)) << "text";
        CHECK(result == expected.str());
    }

    SUBCASE("AbsoluteStyle") {
        result = std::format("{}text", abs(bold | fg(yellow)));
        expected << abs(bold | fg(yellow)) << "text";
        CHECK(result == expected.str());
    }

    SUBCASE("reset") {
        result = std::format("{}text{}", bold, reset);
        expected << bold << "text" << reset;
        CHECK(result == expected.str());
    }

    SUBCASE("styled") {
        result = std::format("{:04}", styled(42, bold));
        expected << abs(bold) << "0042" << reset;
        CHECK(result == expected.str());
    }
}

// TODO: 
# if 0

TEST_CASE("StyledPrint" * test_suite("StyledPrint")) {
    std::ostringstream os;
    std::ostringstream expected;
    StyledPrint out;
    out.set_stream(os).set_base_style(fg(blue));

    SUBCASE("plain values") {
        out.print("{} {}", "text", 42).print(" {}", "next");
        expected << abs(fg(blue)) << "text 42 next";
    }

    SUBCASE("with style specified") {
        out.print(bold, "{}", "bold").print("{}", "base");
        expected << abs(fg(blue)) << bold << "bold" << abs(fg(blue)) << "base";
    }

    SUBCASE("writes Styled") {
        out.print(fg(red), "before{}after", italic("italic"));
        expected << abs(fg(blue)) << fg(red) << "before" << italic << "italic"
                 << abs(fg(red)) << "after" << abs(fg(blue));
    }

    SUBCASE("style disabled") {
        out.enable_style(false);
        out.print(fg(red), "A{}B", bold("x"));
        expected << "AxB";
    }

    CHECK(os.str() == expected.str());
}

# endif

// NOLINTEND

