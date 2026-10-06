#include <decoterm/format.hpp>

#include "decoterm/output.hpp"
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
        result = std::format("{:04}{}", styled(42, bold), styled("hello", italic));
        expected << bold << "0042" << reset << italic << "hello" << reset;
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

    SUBCASE("styled() restore to specified style") {
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
    std::ostringstream os;
    std::string expected;

    SUBCASE("print") {
        print(os, bold, "bold text");
        expected = std::format("{}bold text{}", bold, reset);
        CHECK(os.str() == expected);
    }

    SUBCASE("println") {
        println(os, bold, "bold text");
        expected = std::format("{}bold text{}\n", bold, reset);
        CHECK(os.str() == expected);
    }
}


TEST_CASE("Printer prints values" * test_suite("Printer")) {
    OutputConfig ocfg;
    std::ostringstream os;
    std::string expected;

    const Style base = italic;
    Printer out(ocfg, os);
    out.set_base_style(base);

    SUBCASE("values") {
        out.print("hello, {}!", "world");
        expected = std::format("{}hello, world!", abs(base));
        CHECK(os.str() == expected);
    }

    SUBCASE("styled") {
        out.print("The answer is {}.", fg(blue) | bold | 42);
        expected = std::format("{}The answer is {}42{}.", abs(base), fg(blue) | bold, abs(base));
        CHECK(os.str() == expected);
    }

    SUBCASE("with style specified") {
        out.println(bg(blue) | fg(black), "TEXT");
        expected = std::format("{}TEXT{}\n", abs(base | bg(blue) | fg(black)), abs(base));
        CHECK(out.current_style() == base);
        CHECK(os.str() == expected);

        os.str("");
        os.clear();

        out.println(bold, "first {} third", fg(blue) | "second");
        expected = std::format("{}first {}second{} third{}\n", bold, fg(blue), abs(base | bold), abs(base));
        CHECK(out.current_style() == base);
        CHECK(os.str() == expected);
    }

}

TEST_CASE("Printer follows OutputConfig" * test_suite("Printer")) {
    OutputConfig ocfg;

    std::ostringstream os;
    std::string expected;
    Printer out(ocfg, os);

    SUBCASE("disable color") {
        ocfg.enable_color(false);

        out .set_base_style(dim)
            .print(bold, "text{}text", fg(blue) | italic | "text");
        expected = std::format("{}text{}text{}text{}", abs(dim | bold), italic, abs(dim | bold), abs(dim));
        CHECK(os.str() == expected);
    }
    
    SUBCASE("disable style") {
        ocfg.enable_style(false);
        out .set_base_style(dim)
            .print(bold, "text{}text", fg(blue) | italic | "text");
        CHECK(os.str() == "texttexttext");
    }


    SUBCASE("color fallback") {

        // TODO: Color16, Color256 fallback
    }
}

TEST_CASE("Printer without OutputConfig writes styles directly" * test_suite("Printer")) {
    std::ostringstream os;
    Printer out(os);

    out.set_base_style(dim)
        .print(bold, "text{}text", fg(blue) | italic | "styled");

    const auto expected = std::format("{}text{}styled{}text{}",
                                      abs(dim | bold),
                                      fg(blue) | italic,
                                      abs(dim | bold),
                                      abs(dim));
    CHECK(os.str() == expected);
}

TEST_CASE("Printer clone_with starts from the cloned style state" * test_suite("Printer")) {
    std::ostringstream source_os;
    std::ostringstream clone_os;
    Printer source(source_os);

    source.set_base_style(bold).push(italic);
    auto clone = source.clone_with(clone_os);

    clone.print("nested").pop().print("base");

    const auto expected = std::format("{}nested{}base",
                                      abs(bold | italic),
                                      abs(bold));
    CHECK(source_os.str().empty());
    CHECK(clone_os.str() == expected);
    CHECK(source.current_style() == (bold | italic));
    CHECK(clone.current_style() == bold);
}

TEST_CASE("Printer style operations" * test_suite("Printer")) {
    OutputConfig ocfg;
    std::ostringstream os;
    Printer out(ocfg, os);

    out.set_base_style(bold);
    CHECK(out.base_style() == bold);
    CHECK(out.current_style() == bold);

    out.apply(dim);
    CHECK(out.current_style() == (bold | dim));
    CHECK(out.base_style() == bold);

    out.push(italic);
    CHECK(out.current_style() == (bold | dim | italic));

    out.push(fg(red));
    CHECK(out.current_style() == (bold | dim | italic | fg(red)));

    out.apply(underline);
    CHECK(out.current_style() == (bold | dim | italic | fg(red) | underline));

    out.pop();
    CHECK(out.current_style() == (bold | dim | italic));

    out.reset();
    CHECK(out.current_style() == bold);

    CHECK(os.str().empty());
}

TEST_CASE("Printer merges styles" * test_suite("Printer")) {
    SUBCASE("merge pending and specified style") {
        OutputConfig ocfg;
        std::ostringstream os;
        Printer out(ocfg, os);

        out.set_base_style(bold)
           .apply(italic).print(fg(red), "text");

        const auto expected = std::format(
            "{}text{}", abs(bold | italic | fg(red)), abs(bold | italic));
        CHECK(out.current_style() == (bold | italic));
        CHECK(os.str() == expected);
    }

    SUBCASE("merge styles across push, pop, and reset") {
        OutputConfig ocfg;
        std::ostringstream os;
        Printer out(ocfg, os);

        out.apply(bold)
            .apply(fg(red))
            .print("one")
            .push(underline)
            .apply(bg(blue))
            .print("two")
            .pop()
            .print("three")
            .reset()
            .print("four");

        const auto expected = std::format("{}one{}two{}three{}four",
                                           abs(bold | fg(red)),
                                           underline | bg(blue),
                                          abs(bold | fg(red)),
                                          reset);
        CHECK(out.current_style() == null_style);
        CHECK(os.str() == expected);
    }
}

// NOLINTEND
