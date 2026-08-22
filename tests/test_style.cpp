#include <decoterm/style.hpp>

#include "unit_test.hpp"
#include <doctest.h>

#include <array>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <iostream>

// NOLINTBEGIN

using namespace deco;
using doctest::test_suite;

auto sgr(std::string_view params) -> std::string {
    std::string result = "\x1b[";
    result += params;
    result += 'm';
    return result;
}


TEST_CASE("Color construct" * test_suite("Color")) {
    SUBCASE("from index") {
        constexpr Color c = 7;
        CHECK(c.data()[0] == 7);
        CHECK(c.type() == ColorType::terminal_color);
        CHECK(c == white);
        CHECK(c);
        CHECK_FALSE(c.is_null());
    }

    SUBCASE("from rgb") {
        constexpr Color c = Color(16, 32, 64);
        auto [r, g, b] = c.data();
        CHECK(r == 16);
        CHECK(g == 32);
        CHECK(b == 64);
        CHECK(c.type() == ColorType::true_color);
        CHECK(c == rgb(16, 32, 64));

        CHECK(rgb(0x123456) == rgb(0x12, 0x34, 0x56));
        CHECK(rgb(0xabcdef) == rgb(0xab, 0xcd, 0xef));
        CHECK(rgb(0xffffff) == rgb(255, 255, 255));
    }

    SUBCASE("default color") {
        constexpr Color c = Color::default_color();
        CHECK(c.type() == ColorType::default_color);
        CHECK(c == default_color);
        CHECK(c);
    }

    SUBCASE("null color") {
        constexpr Color c = Color::null_color();
        CHECK(c.type() == ColorType::null);
        CHECK(c == null_color);
        CHECK_FALSE(c);
        CHECK(c.is_null());
    }

    SUBCASE("from hsv()") {
        CHECK(hsv(0, 255, 255) == rgb(255, 0, 0));
        CHECK(hsv(60, 255, 255) == rgb(255, 255, 0));
        CHECK(hsv(120, 255, 255) == rgb(0, 255, 0));
        CHECK(hsv(180, 255, 255) == rgb(0, 255, 255));
        CHECK(hsv(240, 255, 255) == rgb(0, 0, 255));
        CHECK(hsv(300, 255, 255) == rgb(255, 0, 255));
        CHECK(hsv(30, 128, 128) == rgb(128, 96, 64));
        CHECK(hsv(359, 0, 42) == rgb(42, 42, 42));

        CHECK_THROWS_AS(static_cast<void>(hsv(360, 255, 255)),
                        std::invalid_argument);
    }
}

TEST_CASE("Color generates escape sequence" * test_suite("Color")) {
    SUBCASE("system colors") {
        for (std::size_t i = 0; i < 16; ++i) {
            CAPTURE(i);
            const Color color = Color(i);
            CHECK(color.to_escape(false) == sgr(detail::SGR_PARAM_FG[i]));
            CHECK(color.to_escape(true) == sgr(detail::SGR_PARAM_BG[i]));
        }
    }

    SUBCASE("xterm non-system colors") {
        constexpr std::array<uint8_t, 8> indexes {
            16, 17, 52, 123, 196, 231, 232, 255};

        for (uint8_t index : indexes) {
            CAPTURE(index);
            const Color c = Color(index);
            std::string index_str = std::to_string(index);
            CHECK(c.to_escape(false) == sgr(std::string("38;5;") + index_str));
            CHECK(c.to_escape(true) == sgr(std::string("48;5;" + index_str)));
        }
    }

    SUBCASE("true color") {
        constexpr Color c = rgb(12, 34, 56);
        std::string rgb_params = "12;34;56";
        CHECK(c.to_escape(false) == sgr(std::string("38;2;") + rgb_params));
        CHECK(c.to_escape(true) == sgr(std::string("48;2;") + rgb_params));
    }

    SUBCASE("default color") {
        CHECK(default_color.to_escape(false) == sgr("39"));
        CHECK(default_color.to_escape(true) == sgr("49"));
    }

    SUBCASE("null color") {
        CHECK(null_color.to_escape(false) == sgr(""));
        CHECK(null_color.to_escape(true) == sgr(""));
    }

    SUBCASE("maximum escape sequence size") {
        const Color c = rgb(123, 101, 123);
        CAPTURE(c);
        CHECK(c.to_escape(true).size() == Color::MAX_ESCAPE_SEQ_SIZE);
    }
}

TEST_CASE("Color debug string" * test_suite("Color")) {
    CHECK(Color(145).debug_string() == std::string("[terminal_color: 145]"));
    CHECK(Color(12, 34, 56).debug_string()
          == std::string("[true_color: 12, 34, 56]"));
    CHECK(default_color.debug_string() == std::string("[default_color]"));
    CHECK(null_color.debug_string() == std::string("[null_color]"));
}

TEST_CASE("Color predefined constants" * test_suite("Color")) {
    constexpr std::array constants {
        black,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white,
        bright_black,
        bright_red,
        bright_green,
        bright_yellow,
        bright_blue,
        bright_magenta,
        bright_cyan,
        bright_white,
    };

    for (std::size_t i = 0; i < constants.size(); ++i) {
        CHECK(constants[i].to_escape(false) == sgr(detail::SGR_PARAM_FG[i]));
        CHECK(constants[i].to_escape(true) == sgr(detail::SGR_PARAM_BG[i]));
    }
}

TEST_CASE("Style construct" * test_suite("Style")) {
    {
        const Style s = Style(0b11001011, black, rgb(64, 128, 32));
        CHECK(s.emphasis() == 0b11001011);
        CHECK(s.fg() == black);
        CHECK(s.bg() == rgb(64, 128, 32));
        CHECK_FALSE(s.is_null());
    }

    {
        const Style s = Style();
        CHECK(s == null_style);
        CHECK(s.emphasis() == 0);
        CHECK(s.fg() == null_color);
        CHECK(s.bg() == null_color);
        CHECK(s.is_null());
    }
}

TEST_CASE("Style generates escape sequence" * test_suite("Style")) {
    std::string buf;
    auto out = std::back_inserter(buf);

    SUBCASE("null style") {
        null_style.to_sgr_params(out);
        CHECK(buf.empty());
        CHECK(null_style.to_escape().empty());
    }

    SUBCASE("emphasis") {
        const Style s = bold | dim | italic | underline | blink | invert
                        | strikethrough | underline_double;
        s.to_sgr_params(out);

        CHECK(buf == "1;2;3;4;5;7;9;21");
        CHECK(s.to_escape() == sgr(buf));
    }

    SUBCASE("only color") {
        const Style s = fg(rgb(1, 2, 3)) | bg(rgb(4, 5, 6));
        s.to_sgr_params(out);
        CHECK(buf == "38;2;1;2;3;48;2;4;5;6");
    }

    SUBCASE("mixed") {
        const Style s = (italic | bold | bg(blue) | underline | fg(red));
        s.to_sgr_params(out);

        CHECK(buf == "31;44;1;3;4");
        CHECK(s.to_escape() == sgr(buf));
    }

    SUBCASE("maximum escape sequence size") {
        const Style s = bold | dim | italic | underline | blink | invert
                        | strikethrough | underline_double
                        | fg(rgb(111, 111, 111)) | bg(rgb(222, 222, 222));

        CAPTURE(s);
        CHECK(s.to_escape().size() == Style::MAX_ESCAPE_SEQ_SIZE);
    }

    SUBCASE("to_escape(out) writes escape sequence to output iterator") {
        const Style s = (italic | bold | bg(blue) | underline | fg(red));
        s.to_escape(out);

        CHECK(buf == s.to_escape());
    }

    if (!buf.empty()) CHECK(buf.back() != ';');
}

TEST_CASE("Style composition" * test_suite("Style")) {
    Style s = bold | italic | fg(red) | bg(blue);

    SUBCASE("emphasis") {
        s |= dim;

        CHECK(s.emphasis() == (Style::bold | Style::italic | Style::dim));
        CHECK(s.fg() == red);
        CHECK(s.bg() == blue);
    }

    SUBCASE("null color doesn't override") {
        s |= fg(null_color) | bg(null_color);

        CHECK(s.fg() == red);
        CHECK(s.bg() == blue);
    }

    SUBCASE("rhs overrides fg") {
        s |= fg(yellow);
        CHECK(s.fg() == yellow);
        CHECK(s.bg() == blue);
    }

    SUBCASE("rhs overrides bg") {
        s |= bg(yellow);
        CHECK(s.fg() == red);
        CHECK(s.bg() == yellow);
    }
}

TEST_CASE("Style debug string" * test_suite("Style")) {
    const Style s = bold | underline | strikethrough | color(cyan, magenta);
    CAPTURE(s.debug_string());
    CHECK(s.debug_string()
          == std::string("[flags=01001001, fg=") + cyan.debug_string()
                 + ", bg=" + magenta.debug_string() + "]");
}

TEST_CASE("AbsoluteStyle is never null" * test_suite("AbsoluteStyle")) {
    AbsoluteStyle a = abs(null_style);
    CHECK_FALSE(a.is_null());
}

TEST_CASE("AbsoluteStyle writes escape sequence"
          * test_suite("AbsoluteStyle")) {
    SUBCASE("null style") { CHECK(abs(null_style).to_escape() == "\x1b[m"); }

    SUBCASE("style") {
        std::string buf;
        const Style s = (italic | bold | bg(blue) | underline | fg(red));
        const AbsoluteStyle a = abs(s);

        s.to_sgr_params(std::back_inserter(buf));
        CHECK(a.to_escape() == "\x1b[;" + buf + "m");
    }

    SUBCASE("maximum escape sequence size") {
        const Style s = bold | dim | italic | underline | blink | invert
                        | strikethrough | underline_double
                        | fg(rgb(111, 111, 111)) | bg(rgb(222, 222, 222));

        CAPTURE(abs(s));
        CHECK(abs(s).to_escape().size() == AbsoluteStyle::MAX_ESCAPE_SEQ_SIZE);
    }
}

class CustomClass {
  public:
    CustomClass(int init) : value_(init) {}

    void increment() { value_++; }
    auto value() const -> int { return value_; }

  private:
    int value_;
};

std::ostream& operator<<(std::ostream& os, const CustomClass& cc) {
    os << cc.value();
    return os;
}

TEST_CASE("Styled stores multiple values" * test_suite("Styled")) {
    SUBCASE("styled() stores rvalue") {
        // 虽说static_assert没必要写进TEST_CASE甚至是SUBCASE里，但是个人
        // 比较偏向于用TEST_CASE和SUBCASE来进行分类，而不单单是为了利用SUBCASE
        // 来减少初始化，因此直接在这里写了。当然如果你有更好的写法可以修改。

        int i = 42;
        const int ci = 84;

        auto styled = deco::styled(bold,
                                   13,
                                   4.5,
                                   std::move(i),
                                   std::move(ci),
                                   &i,
                                   &ci,
                                   CustomClass(42));

        static_assert(std::is_same_v<decltype(styled),
                                     Styled<Style,
                                            int,
                                            double,
                                            int,
                                            int,
                                            int*,
                                            const int*,
                                            CustomClass>>);

        SUBCASE("output") {
            //outputs using operator<<
            // 可能用stringstream去比较？
        }
    }

    SUBCASE("styled() borrows lvalue by using ConstRef") {
        SUBCASE("output") {
        }
    }

    SUBCASE("styled() decays array and functions") {
        // string literals, normal arrays, functions...
        SUBCASE("output") {
        }
    }

    SUBCASE("nested Styled") {
        SUBCASE("output") {
        }
    }
}

TEST_CASE("construct Styled by functional Style") {
}

// NOLINTEND
