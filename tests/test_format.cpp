#include <decoterm/format.hpp>
#include <doctest.h>

#include <format>
#include <iterator>
#include <sstream>
#include <string>
#include <version>


// NOLINTBEGIN

TEST_CASE("format: formatters") {
    using namespace deco;

    CHECK(std::format("{}text{}", bold, reset)
          == bold.to_escape() + std::string("text")
                 + abs(null_style).to_escape());
    CHECK(std::format("{}", styled(fg(red), std::string("text")))
          == fg(red).to_escape() + std::string("text")
                 + abs(null_style).to_escape());
    CHECK(std::format("{:04}", styled(bold, 42))
          == bold.to_escape() + std::string("0042")
                 + abs(null_style).to_escape());
}

TEST_CASE("format: StyledFormat: format_to()") {
    using namespace deco;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    std::string out;

    sfmt.format_to(std::back_inserter(out), "{} {}", "value", 42);
    sfmt.format_to(std::back_inserter(out), " {}", "next");

    CHECK(out
          == abs(fg(white)).to_escape() + std::string("value 42 next"));
}

TEST_CASE("format: StyledFormat: format_to() with style") {
    using namespace deco;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    std::string out;

    sfmt.format_to(std::back_inserter(out), bold, "{}", "bold");
    sfmt.format_to(std::back_inserter(out), "{}", "base");

    CHECK(out
          == abs(fg(white)).to_escape() + bold.to_escape()
                 + std::string("bold") + abs(fg(white)).to_escape()
                 + std::string("base"));
}

TEST_CASE("format: StyledFormat: format()") {
    using namespace deco;

    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    CHECK(sfmt.format("{}", "base")
          == abs(fg(white)).to_escape() + std::string("base"));
    CHECK(sfmt.format(italic, "{}", "italic")
          == italic.to_escape() + std::string("italic")
                 + abs(fg(white)).to_escape());
}

TEST_CASE("format: StyledFormat: format() with StyledRef") {
    using namespace deco;

    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    CHECK(sfmt.format("base{}base", styled(bold, std::string("bold")))
          == abs(fg(white)).to_escape() + std::string("base")
                 + bold.to_escape() + std::string("bold")
                 + abs(fg(white)).to_escape() + std::string("base"));
}

TEST_CASE("format: StyledFormat: StyledRef restores call style") {
    using namespace deco;

    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));

    CHECK(
        sfmt.format(fg(red), "before{}after", styled(bold, std::string("bold")))
        == abs(fg(white)).to_escape() + fg(red).to_escape()
               + std::string("before") + bold.to_escape() + std::string("bold")
               + abs(fg(red)).to_escape() + std::string("after")
               + abs(fg(white)).to_escape());
}

TEST_CASE("format: StyledFormat: StyledRef restores absolute call style") {
    using namespace deco;

    const Style base = fg(white) | bold;
    StyledFormat sfmt = StyledFormat().set_base_style(base);

    CHECK(sfmt.format(abs(fg(red)),
                      "before{}after",
                      styled(italic, std::string("italic")))
          == abs(base).to_escape() + abs(fg(red)).to_escape()
                 + std::string("before") + italic.to_escape()
                 + std::string("italic") + abs(fg(red)).to_escape()
                 + std::string("after") + abs(base).to_escape());
}

TEST_CASE("format: StyledFormat: disabled style with StyledRef") {
    using namespace deco;

    StyledFormat sfmt;
    sfmt.enable_style(false);

    CHECK(sfmt.format(fg(red), "A{}B", styled(bold, std::string("x")))
          == "AxB");
}

# if 0

TEST_CASE("format: StyledPrint: print()" * doctest::may_fail()) {
    using namespace deco;

    std::ostringstream os;
    StyledPrint sfmt = StyledPrint().set_base_style(fg(white));
    sfmt.set_stream(os);

    sfmt.print("{} {}", "value", 42).print(" {}", "next");

    CHECK(os.str()
          == abs(fg(white)).to_escape() + std::string("value 42 next"));
}

TEST_CASE("format: StyledPrint: print() with style") {
    using namespace deco;

    std::ostringstream os;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));
    sfmt.set_stream(os);

    sfmt.print(bold, "{}", "bold").print("{}", "base");

    CHECK(os.str()
          == abs(fg(white)).to_escape() + bold.to_escape()
                 + std::string("bold") + abs(fg(white)).to_escape()
                 + std::string("base"));
}

TEST_CASE("format: StyledFormat: print() with StyledRef") {
    using namespace deco;

    std::ostringstream os;
    StyledFormat sfmt = StyledFormat().set_base_style(fg(white));
    sfmt.set_stream(os);

    sfmt.print("base{}base", styled(bold, std::string("bold")));

    CHECK(os.str()
          == abs(fg(white)).to_escape() + std::string("base")
                 + bold.to_escape() + std::string("bold")
                 + abs(fg(white)).to_escape() + std::string("base"));
}

TEST_CASE("format: StyledFormat: print StyledRef context") {
    using namespace deco;

    const Style base = fg(white) | bold;
    std::ostringstream os;
    StyledFormat sfmt = StyledFormat().set_base_style(base);
    sfmt.set_stream(os);

    sfmt.print(abs(fg(red)),
               "before{}after",
               styled(italic, std::string("italic")));

    CHECK(os.str()
          == abs(base).to_escape() + abs(fg(red)).to_escape()
                 + std::string("before") + italic.to_escape()
                 + std::string("italic") + abs(fg(red)).to_escape()
                 + std::string("after") + abs(base).to_escape());
}

TEST_CASE("format: StyledFormat: print disabled style with StyledRef") {
    using namespace deco;

    std::ostringstream os;
    StyledFormat sfmt;
    sfmt.enable_style(false).set_stream(os);

    sfmt.print(fg(red), "A{}B", styled(bold, std::string("x")));

    CHECK(os.str() == "AxB");
}

TEST_CASE("format: StyledFormat: set stream(std::ostream)") {
    using namespace deco;

    std::ostringstream os1;
    std::ostringstream os2;
    StyledFormat sfmt;
    sfmt.enable_style(false);

    sfmt.set_stream(os1).print("{}", "one");
    sfmt.set_stream(os2).print("{}", "two");

    CHECK(os1.str() == "one");
    CHECK(os2.str() == "two");
}

TEST_CASE("format: StyledFormat: style nesting") {
    using namespace deco;

    StyledFormat sfmt =
        StyledFormat().set_base_style(fg(white)).enable_nesting();
    std::string out;

    sfmt.format_to(std::back_inserter(out), "{}", "base");
    sfmt.push(bold).format_to(std::back_inserter(out), "{}", "bold");
    sfmt.push(italic).format_to(std::back_inserter(out), fg(red), "{}", "red");
    sfmt.pop().format_to(std::back_inserter(out), "{}", "bold2");
    sfmt.reset().format_to(std::back_inserter(out), "{}", "base2");

    const Style bold_white = fg(white) | bold;
    const Style italic_bold_white = fg(white) | bold | italic;

    CHECK(out
          == abs(fg(white)).to_escape() + std::string("base")
                 + abs(bold_white).to_escape() + std::string("bold")
                 + abs(italic_bold_white).to_escape() + fg(red).to_escape()
                 + std::string("red") + abs(italic_bold_white).to_escape()
                 + abs(bold_white).to_escape() + std::string("bold2")
                 + abs(fg(white)).to_escape() + std::string("base2"));
}

#endif

// NOLINTEND
