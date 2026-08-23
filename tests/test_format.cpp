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

    SUBCASE("simple Styled") {
        result = std::format("{:04}", styled(bold, 42));
        expected << abs(bold) << "0042" << reset;
        CHECK(result == expected.str());
    }

    SUBCASE("complex Styled") {
        result =
            std::format("{}", styled(fg(blue), "outer", bold("inner"), "tail"));
        expected << abs(fg(blue)) << "outer" << abs(fg(blue) | bold) << "inner"
                 << abs(fg(blue)) << "tail" << reset;
        CHECK(result == expected.str());
    }

}

TEST_CASE("StyledFormat formats values" * test_suite("StyledFormat")) {
    std::string result;
    std::ostringstream expected;
    StyledFormat fmt;
    fmt.set_base_style(fg(blue));

    SUBCASE("format_to") {
        fmt.format_to(std::back_inserter(result), "{} {}", "text", 42);
        expected << abs(fg(blue)) << "text " << 42;
        CHECK(result == expected.str());
    }

    SUBCASE("format_to with style specified") {
        fmt.format_to(std::back_inserter(result), bold, "{}", "bold");
        fmt.format_to(std::back_inserter(result), "{}", "base");

        expected << abs(fg(blue)) << bold << "bold" << abs(fg(blue)) << "base";
        CHECK(result == expected.str());
    }

    SUBCASE("format") {
        result = fmt.format(italic, "{}", "italic");

        expected << abs(fg(blue)) << italic << "italic" << abs(fg(blue));
        CHECK(result == expected.str());
    }

    SUBCASE("nested style state") {
        fmt.enable_nesting();

        fmt.format_to(std::back_inserter(result), "{}", "base");
        fmt.push(bold).format_to(std::back_inserter(result), "{}", "bold");
        fmt.push(italic).format_to(
            std::back_inserter(result), fg(red), "{}", "red");
        fmt.pop().format_to(std::back_inserter(result), "{}", "bold2");
        fmt.reset().format_to(std::back_inserter(result), "{}", "base2");

        const Style bold_blue = fg(blue) | bold;
        const Style italic_bold_blue = fg(blue) | bold | italic;

        expected << abs(fg(blue)) << "base" << abs(bold_blue) << "bold"
                 << abs(italic_bold_blue) << fg(red) << "red"
                 << abs(italic_bold_blue) << abs(bold_blue) << "bold2"
                 << abs(fg(blue)) << "base2";
        CHECK(result == expected.str());
    }

}

TEST_CASE("StyledFormat writes Styled" * test_suite("StyledFormat")) {
    std::string result;
    std::ostringstream expected;
    StyledFormat fmt;
    fmt.set_base_style(fg(blue));

    SUBCASE("simple") {
        result = fmt.format(fg(red), "before{}after", italic("italic"));

        expected << abs(fg(blue)) << fg(red) << "before" << abs(fg(red) | italic) << "italic"
                 << abs(fg(red)) << "after" << abs(fg(blue));
        CHECK(result == expected.str());
    }

    SUBCASE("complex") {
        auto styled = bold("lorem", 42, italic("ipsum"), "dolor");

        result = fmt.format("{}sit", styled);

        expected << abs(fg(blue)) << abs(fg(blue) | bold) << "lorem" << 42
                 << abs(fg(blue) | bold | italic) << "ipsum"
                 << abs(fg(blue) | bold) << "dolor" << abs(fg(blue)) << "sit";
        CHECK(result == expected.str());
    }

    SUBCASE("style disabled") {
        fmt.enable_style(false);

        result = fmt.format(fg(red), "A{}B", bold("x"));

        expected << "AxB";
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
