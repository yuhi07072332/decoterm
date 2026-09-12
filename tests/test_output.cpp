#include <decoterm/output.hpp>
#include <decoterm/style.hpp>

#include "unit_test.hpp"
#include <doctest.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace {

struct TestStyleState : public deco::StyleState,
                        public deco::StyleStateSetter<TestStyleState> {
    using deco::StyleState::pop_style;
    using deco::StyleState::push_style;
    using deco::StyleState::reset_style;
    using deco::StyleState::update_context;

    TestStyleState() = default; // NOLINT

    TestStyleState(const StyleState& other) : StyleState(other) {}

    TestStyleState(StyleState&& other) : StyleState(std::forward<StyleState>(other)) {}
};

struct return_different_ostream_t {};

auto operator<<(std::ostream& os, return_different_ostream_t) // NOLINT
    -> std::ostream& {                                        // NOLINT
    return std::cerr;
}

auto set_width_4(std::basic_ios<char>& ios) -> std::basic_ios<char>& {
    ios.width(4);
    return ios;
}

} // namespace

// NOLINTBEGIN

using namespace deco;
using doctest::test_suite;

TEST_CASE("StyleState options" * test_suite("StyleState")) {
    StyledOstream state(std::cout);

    SUBCASE("defaults") {
        CHECK(state.style_enabled());
        CHECK_FALSE(state.nesting_enabled());
        CHECK(state.color_mode() == ColorMode::true_color);
        CHECK_FALSE(state.context_tracking_enabled());
        CHECK(state.base_style() == null_style);
        CHECK(state.current_style().style == null_style);
    }

    SUBCASE("setter chain") {
        state.enable_style(false)
            .enable_nesting()
            .enable_context_tracking()
            .set_color_mode(ColorMode::color16)
            .set_base_style(fg(blue));

        CHECK_FALSE(state.style_enabled());
        CHECK(state.nesting_enabled());
        CHECK(state.color_mode() == ColorMode::color16);
        CHECK(state.context_tracking_enabled());
        CHECK(state.base_style() == fg(blue));
        CHECK(state.current_style().style == fg(blue));
    }

    SUBCASE("setters after chained construction") {
        auto chained =
            styled_out(std::cout).enable_style(false).set_base_style(fg(blue));

        chained.enable_style(true)
            .enable_context_tracking(false)
            .set_base_style(fg(red));

        CHECK(chained.style_enabled());
        CHECK_FALSE(chained.context_tracking_enabled());
        CHECK(chained.base_style() == fg(red));
        CHECK(chained.current_style().style == fg(red));
    }
}

TEST_CASE("StyleState copy and move" * test_suite("StyleState")) {
    auto styled_os =
        StyledOstream(std::cout).enable_style(false).set_base_style(fg(blue));
    styled_os << bold;

    SUBCASE("copy constructs from state") {
        TestStyleState state(styled_os);

        CHECK_FALSE(state.style_enabled());
        CHECK_FALSE(state.context_tracking_enabled());
        CHECK(state.base_style() == fg(blue));
        CHECK(state.current_style().style == (fg(blue) | bold));
    }

    SUBCASE("move constructs from state") {
        TestStyleState state(std::move(styled_os));

        CHECK_FALSE(state.style_enabled());
        CHECK_FALSE(state.context_tracking_enabled());
        CHECK(state.base_style() == fg(blue));
        CHECK(state.current_style().style == (fg(blue) | bold));
    }

    SUBCASE("copy assignment clears stale context tracking") {
        TestStyleState state;
        state.enable_context_tracking();
        state.update_context();

        state = styled_os;

        CHECK(detail::g_style_output_context == nullptr);
    }
}


TEST_CASE("StyleState manages nested styles" * test_suite("StyleState")) {
    TestStyleState state;
    state.set_base_style(fg(blue)).enable_nesting();

    SUBCASE("push, pop, and reset") {
        CHECK(state.current_style().style == fg(blue));

        state.push_style(bold);
        CHECK(state.current_style().style == (fg(blue) | bold));

        state.push_style(italic);
        CHECK(state.current_style().style == (fg(blue) | bold | italic));

        state.push_style(fg(red));
        CHECK(state.current_style().style == (fg(red) | bold | italic));

        state.pop_style();
        CHECK(state.current_style().style == (fg(blue) | bold | italic));

        state.pop_style();
        CHECK(state.current_style().style == (fg(blue) | bold));

        state.pop_style();
        CHECK(state.current_style().style == fg(blue));

        state.pop_style();
        CHECK(state.current_style().style == fg(blue));

        state.push_style(bold);
        state.reset_style();
        CHECK(state.current_style().style == fg(blue));
    }

    SUBCASE("StyleStack grows to heap") {
        for (int i = 0; i < 5; ++i) {
            state.push_style(fg(i));
        }
        state.push_style(italic);

        CHECK(state.current_style().style == (fg(4) | italic));
        for (int i = 4; i >= 0; --i) {
            state.pop_style();
            CHECK(state.current_style().style == fg(i));
        }
    }

    SUBCASE("StyleStack grows more than once") {
        for (int i = 0; i < 12; ++i) {
            state.push_style(fg(i));
            CHECK(state.current_style().style == fg(i));
        }

        for (int i = 10; i >= 0; --i) {
            state.pop_style();
            CHECK(state.current_style().style == fg(i));
        }
    }
}

TEST_CASE("StyleState copies and moves nested stacks"
          * test_suite("StyleState")) {
    TestStyleState source;
    source.set_base_style(fg(blue)).enable_nesting();

    SUBCASE("move preserves local StyleStack") {
        source.push_style(bold);
        source.push_style(italic);

        TestStyleState moved(std::move(source));

        CHECK(moved.current_style().style == (fg(blue) | bold | italic));

        source.push_style(fg(red));
        CHECK(source.current_style().style == fg(red));
    }

    SUBCASE("move preserves heap StyleStack") {
        for (int i = 0; i < 6; ++i) {
            source.push_style(fg(i));
        }

        TestStyleState moved(std::move(source));

        CHECK(moved.current_style().style == fg(5));

        source.push_style(fg(red));
        CHECK(source.current_style().style == fg(red));
    }

    SUBCASE("copy assigns local StyleStack") {
        source.push_style(bold);

        TestStyleState target;
        target.set_base_style(fg(red));
        target = source;

        CHECK(target.current_style().style == (fg(blue) | bold));

        target.push_style(italic);
        CHECK(target.current_style().style == (fg(blue) | bold | italic));
        CHECK(source.current_style().style == (fg(blue) | bold));
    }

    SUBCASE("copy assigns heap StyleStack") {
        for (int i = 0; i < 6; ++i) {
            source.push_style(fg(i));
        }

        TestStyleState target;
        target = source;

        CHECK(target.current_style().style == fg(5));

        target.push_style(italic);
        CHECK(target.current_style().style == (fg(5) | italic));
        CHECK(source.current_style().style == fg(5));
    }
}

TEST_CASE("StyledOstream writes values" * test_suite("StyledOstream")) {
    std::ostringstream os;
    std::ostringstream expected;
    StyledOstream styled_os(os);

    SUBCASE("plain values") {
        double d = 1.5;

        styled_os << "value=" << 42 << ' ' << d;
        expected << abs(null_style) << "value=42 1.5";
    }

    SUBCASE("IO manipulators") {
        styled_os << std::boolalpha << true << ' ' << std::noboolalpha << false
                  << ' ' << std::showbase << std::hex << 42 << ' '
                  << std::noshowbase << std::dec << 42 << ' ' << std::uppercase
                  << std::scientific << std::setprecision(3) << 1.5 << ' '
                  << std::nouppercase << std::fixed << std::setprecision(2)
                  << 1.5 << ' ' << std::defaultfloat << std::showpos << 7 << ' '
                  << std::noshowpos << std::showpoint << 2.0 << ' '
                  << std::noshowpoint << std::setfill('.') << std::left
                  << std::setw(5) << 12 << ' ' << std::right << std::setw(5)
                  << 12 << ' ' << std::internal << std::showpos << std::setw(5)
                  << 12 << std::noshowpos << ' ' << set_width_4 << 9
                  << std::endl
                  << std::flush << std::ends;

        expected << abs(null_style) << std::boolalpha << true << ' '
                 << std::noboolalpha << false << ' ' << std::showbase
                 << std::hex << 42 << ' ' << std::noshowbase << std::dec << 42
                 << ' ' << std::uppercase << std::scientific
                 << std::setprecision(3) << 1.5 << ' ' << std::nouppercase
                 << std::fixed << std::setprecision(2) << 1.5 << ' '
                 << std::defaultfloat << std::showpos << 7 << ' '
                 << std::noshowpos << std::showpoint << 2.0 << ' '
                 << std::noshowpoint << std::setfill('.') << std::left
                 << std::setw(5) << 12 << ' ' << std::right << std::setw(5)
                 << 12 << ' ' << std::internal << std::showpos << std::setw(5)
                 << 12 << std::noshowpos << ' ' << set_width_4 << 9 << std::endl
                 << std::flush << std::ends;
    }

    CHECK(os.str() == expected.str());
}

TEST_CASE("StyledOstream handles errors" * test_suite("StyledOstream")) {
    StyledOstream styled_os(std::cout);

    SUBCASE("output operator returns different ostream object") {
        CHECK_THROWS_AS(styled_os << return_different_ostream_t {},
                        std::logic_error);
    }
}

TEST_CASE("StyledOstream writes styles" * test_suite("StyledOstream")) {
    std::ostringstream os;
    std::ostringstream expected;
    StyledOstream styled_os(os);

    SUBCASE("style output") {
        styled_os << fg(red) << "error" << reset;
        expected << abs(null_style) << fg(red) << "error" << reset;
    }

    SUBCASE("disabled style output") {
        styled_os.enable_style(false);

        styled_os << fg(red) << "error" << reset;
        expected << "error";

        CHECK(styled_os.current_style().style == null_style);
    }

    CHECK(os.str() == expected.str());
}

TEST_CASE("StyledOstream tracks current style" * test_suite("StyledOstream")) {
    std::ostringstream os;
    auto out = styled_out(os).set_base_style(fg(blue));

    SUBCASE("nested styles restore previous style") {
        out.enable_nesting();

        out << bold << "bold" << fg(red) << "red bold" << pop << "blue bold"
            << pop << "blue";

        CHECK(os.str()
              == abs(fg(blue)).to_escape() + bold.to_escape()
                     + std::string("bold") + fg(red).to_escape()
                     + std::string("red bold")
                     + abs(fg(blue) | bold).to_escape()
                     + std::string("blue bold") + abs(fg(blue)).to_escape()
                     + std::string("blue"));
    }

    SUBCASE("reset returns to base style") {
        out << bold << "bold" << reset << "base";

        CHECK(os.str()
              == abs(fg(blue)).to_escape() + bold.to_escape()
                     + std::string("bold") + abs(fg(blue)).to_escape()
                     + std::string("base"));
    }

    SUBCASE("context tracking keeps independent streams") {
        std::ostringstream other_os;
        auto other = styled_out(other_os).set_base_style(fg(red));
        std::ostringstream expected_os, expected_other_os;

        out << "a" << "b";
        other << "c";
        out << "d" << styled(bold, "e") << "f";
        other << "g";

        expected_os << abs(fg(blue)) << "ab" << abs(fg(blue))
            << "d" << abs(fg(blue) | bold) << "e" << abs(fg(blue)) << "f";
        expected_other_os << abs(fg(red)) << "c" << abs(fg(red)) << "g";

        CHECK(os.str() == expected_os.str());
        CHECK(other_os.str() == expected_other_os.str());
    }

    SUBCASE("base style is output only once without context tracking") {
        out = StyledOstream(os).set_base_style(fg(blue));

        out << "a" << "b" << bold << "c" << pop << "d" << reset << "e";

        CHECK(os.str()
              == abs(fg(blue)).to_escape() + std::string("ab")
                     + bold.to_escape() + std::string("c")
                     + abs(fg(blue)).to_escape() + std::string("d")
                     + abs(fg(blue)).to_escape() + std::string("e"));
    }
}

TEST_CASE("StyledOstream writes Styled" * test_suite("StyledOstream")) {
    std::ostringstream os;
    std::ostringstream expected;
    const Style base = fg(blue);
    auto out = styled_out(os).set_base_style(base);

    auto styled_values = styled(bold, "lorem", 42, italic % "ipsum", "dolor");
    out << styled_values << "sit";
    expected << abs(base) << abs(base | bold) << "lorem" << 42
             << abs(base | bold | italic) << "ipsum" << abs(base | bold) << "dolor"
             << abs(base) << "sit";

    CHECK(os.str() == expected.str());
}

// NOLINTEND
