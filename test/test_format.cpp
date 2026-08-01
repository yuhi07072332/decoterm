#include <decoterm/format.hpp>
#include <doctest.h>

#include <format>
#include <iterator>
#include <sstream>
#include <string>
#include <version>

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202403L
#define DECO_ENABLE_PRINT 1 // NOLINT
#else
#define DECO_ENABLE_PRINT 0
#endif

// NOLINTBEGIN

TEST_CASE("format: formatters") {
    using namespace deco;

    CHECK(std::format("{}text{}", bold, reset)
          == bold.to_escape() + std::string("text")
                 + absolute(null_style).to_escape());
    CHECK(std::format("{}", styled(std::string("text"), fg(red)))
          == fg(red).to_escape() + std::string("text")
                 + absolute(null_style).to_escape());
    CHECK(std::format("{:04}", styled(42, bold))
          == bold.to_escape() + std::string("0042")
                 + absolute(null_style).to_escape());
}

TEST_CASE("format: StyledFormat: format_to()") {
    using namespace deco;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    std::string out;

    sfmt.format_to(std::back_inserter(out), "{} {}", "value", 42);
    sfmt.format_to(std::back_inserter(out), " {}", "next");

    CHECK(out
          == absolute(fg(white)).to_escape() + std::string("value 42 next"));
}

TEST_CASE("format: StyledFormat: format_to() with style") {
    using namespace deco;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    std::string out;

    sfmt.format_to(std::back_inserter(out), bold, "{}", "bold");
    sfmt.format_to(std::back_inserter(out), "{}", "base");

    CHECK(out
          == absolute(fg(white)).to_escape() + bold.to_escape()
                 + std::string("bold") + absolute(fg(white)).to_escape()
                 + std::string("base"));
}

TEST_CASE("format: StyledFormat: format()") {
    using namespace deco;

    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    CHECK(sfmt.format("{}", "base")
          == absolute(fg(white)).to_escape() + std::string("base"));
    CHECK(sfmt.format(italic, "{}", "italic")
          == italic.to_escape() + std::string("italic")
                 + absolute(fg(white)).to_escape());
}

TEST_CASE("format: StyledFormat: print()") {
#if DECO_ENABLE_PRINT
    using namespace deco;

    std::ostringstream os;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));
    sfmt.set_stream(os);

    sfmt.print("{} {}", "value", 42).print(" {}", "next");

    CHECK(os.str()
          == absolute(fg(white)).to_escape() + std::string("value 42 next"));
#else
    CHECK(true);
#endif
}

TEST_CASE("format: StyledFormat: print() with style") {
#if DECO_ENABLE_PRINT
    using namespace deco;

    std::ostringstream os;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));
    sfmt.set_stream(os);

    sfmt.print(bold, "{}", "bold").print("{}", "base");

    CHECK(os.str()
          == absolute(fg(white)).to_escape() + bold.to_escape()
                 + std::string("bold") + absolute(fg(white)).to_escape()
                 + std::string("base"));
#else
    CHECK(true);
#endif
}

TEST_CASE("format: StyledFormat: set stream(std::ostream)") {
#if DECO_ENABLE_PRINT
    using namespace deco;

    std::ostringstream os1;
    std::ostringstream os2;
    StyledFormat sfmt;
    sfmt.enable_style(false);

    sfmt.set_stream(os1).print("{}", "one");
    sfmt.set_stream(os2).print("{}", "two");

    CHECK(os1.str() == "one");
    CHECK(os2.str() == "two");
#else
    CHECK(true);
#endif
}

TEST_CASE("format: StyledFormat: style nesting") {
    using namespace deco;

    StyledFormat sfmt =
        StyledFormat().set_base_style(fg(white)).enable_nesting(true);
    std::string out;

    sfmt.format_to(std::back_inserter(out), "{}", "base");
    sfmt.push(bold).format_to(std::back_inserter(out), "{}", "bold");
    sfmt.push(italic).format_to(std::back_inserter(out), fg(red), "{}", "red");
    sfmt.pop().format_to(std::back_inserter(out), "{}", "bold2");
    sfmt.reset().format_to(std::back_inserter(out), "{}", "base2");

    const Style bold_white = fg(white) | bold;
    const Style italic_bold_white = fg(white) | bold | italic;

    CHECK(out
          == absolute(fg(white)).to_escape() + std::string("base")
                 + absolute(bold_white).to_escape() + std::string("bold")
                 + absolute(italic_bold_white).to_escape() + fg(red).to_escape()
                 + std::string("red") + absolute(italic_bold_white).to_escape()
                 + absolute(bold_white).to_escape() + std::string("bold2")
                 + absolute(fg(white)).to_escape() + std::string("base2"));
}

// NOLINTEND
