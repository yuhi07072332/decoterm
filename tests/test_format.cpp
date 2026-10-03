#include <decoterm/format.hpp>

#include "unit_test.hpp"
#include <doctest.h>

#include <format>
#include <sstream>
#include <string>
#include <version>

// NOLINTBEGIN

using namespace deco;
using doctest::test_suite;


TEST_CASE("BufAppender" * test_suite("BufAppender")) {
    static_assert(std::output_iterator<detail::BufAppender<char, 1>, const char&>);

    detail::Buffer<char, 5> buf;

    SUBCASE("without grow") {
        detail::write_to(detail::BufAppender(buf), "hello");
        CHECK(buf.size() == 5);
        CHECK(std::string_view(buf.data(), buf.size())
              == std::string_view("hello"));
    }

    SUBCASE("grow") {
        detail::write_to(detail::BufAppender(buf), "hello world");
        CHECK(buf.size() == 11);
        CHECK(std::string_view(buf.data(), buf.size())
              == std::string_view("hello world"));
    }
}

TEST_CASE("formatters" * test_suite("formatters")) {
    static_assert(std::is_same_v<detail::formatter_t<const char(&) [5]>, std::formatter<std::string_view>>);
    static_assert(std::is_same_v<detail::formatter_t<int (&)[5]>, std::formatter<int[5]>>);

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
        expected << bold << "0042" << reset;
        CHECK(result == expected.str());
    }
}

TEST_CASE("format with style" * test_suite("format")) {
    std::string result;
    std::string expected;

    SUBCASE("Style") {
        result = format(bold, "bold text");
        expected = std::format("{}bold text{}", bold, reset);
        CHECK(result == expected);
    }

    SUBCASE("AbsoluteStyle") {
        result = format(abs(bold), "abs bold text");
        expected =  std::format("{}abs bold text{}", abs(bold), reset);
        CHECK(result == expected);
    }

    SUBCASE("format with null style won't reset") {
        result = format(null_style, "text");
        expected = std::format("text");
        CHECK(result == expected);
    }

    SUBCASE("styled() restore to style") {
        result = format(fg(blue), "blue, {}, blue", styled("bold blue", bold));
        expected = std::format("{}blue, {}bold blue{}, blue{}",
                             fg(blue),
                             bold,
                             abs(fg(blue)),
                             reset);
        CAPTURE(result);
        CAPTURE(expected);
        CHECK(result == expected);
    }
}

TEST_CASE("print with style" * test_suite("print")) {
    std::ostringstream result;
    std::string expected;

    SUBCASE("print") {
        print(result, bold, "bold text");
        expected = std::format("{}bold text{}", bold, reset);
        CHECK(result.str() == expected);
    }

    SUBCASE("println") {
        println(result, bold, "bold text");
        expected = std::format("{}bold text{}\n", bold, reset);
        CHECK(result.str() == expected);
    }
}

#if 0

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

#endif

// NOLINTEND
