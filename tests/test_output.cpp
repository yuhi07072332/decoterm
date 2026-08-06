#include <decoterm/style.hpp>
#include <decoterm/output.hpp>
#include <doctest.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace {

struct TestStyleOutputState
    : public deco::StyleState,
      public deco::StyleStateOption<TestStyleOutputState> {
    using deco::StyleState::pop_style;
    using deco::StyleState::push_style;
    using deco::StyleState::reset_style;

    TestStyleOutputState() = default; // NOLINT

    TestStyleOutputState(const StyleState& other)
        : StyleState(other) {}
};


struct return_dirrerent_ostream_t {};

auto operator<<(std::ostream& os, return_dirrerent_ostream_t) //NOLINT
    -> std::ostream& { // NOLINT
    return std::cerr;
}

auto set_width_4(std::basic_ios<char>& ios) -> std::basic_ios<char>& {
    ios.width(4);
    return ios;
}

} // namespace

// NOLINTBEGIN

TEST_CASE("styled_out: StyleOutputState: copy from other type") {
    using namespace deco;

    auto styled_os = StyledOstream(std::cout)
                         .enable_style(false)
                         .set_base_style(fg(blue))
                         .enable_nesting(true);
    styled_os << bold;

    TestStyleOutputState teststate(styled_os); // NOLINT

    CHECK_FALSE(teststate.style_enabled());
    CHECK_FALSE(teststate.context_enabled());
    CHECK(teststate.nesting_enabled());
    CHECK(teststate.base_style() == fg(blue));
    CHECK(teststate.current_style().style == (fg(blue) | bold));
}

TEST_CASE("styled_out: StyleOutputState: options") {
    using namespace deco;

    StyledOstream state(std::cout);

    CHECK(state.style_enabled());
    CHECK_FALSE(state.context_enabled());
    CHECK_FALSE(state.nesting_enabled());
    CHECK(state.base_style() == null_style);
    CHECK(state.current_style().style == null_style);

    state.enable_style(false).enable_context(true).set_base_style(fg(blue));

    CHECK_FALSE(state.style_enabled());
    CHECK(state.context_enabled());
    CHECK(state.base_style() == fg(blue));
    CHECK(state.current_style().style == fg(blue));

    state.enable_nesting(true);
    CHECK(state.nesting_enabled());
    CHECK(state.current_style().style == fg(blue));

    state.enable_nesting(false);
    CHECK_FALSE(state.nesting_enabled());
    CHECK(state.current_style().style == fg(blue));
}

TEST_CASE("styled_out: StyleOutputState: options after chained construction") {
    using namespace deco;

    auto state = styled_out(std::cout)
                     .enable_style(false)
                     .set_base_style(fg(blue))
                     .enable_nesting(true);

    state.enable_style(true)
        .enable_context(false)
        .set_base_style(fg(red))
        .enable_nesting(false);

    CHECK(state.style_enabled());
    CHECK_FALSE(state.context_enabled());
    CHECK_FALSE(state.nesting_enabled());
    CHECK(state.base_style() == fg(red));
    CHECK(state.current_style().style == fg(red));
}

TEST_CASE("styled_out: StyleOutputState: single style state") {
    using namespace deco;

    TestStyleOutputState state;
    state.set_base_style(fg(blue));

    state.push_style(bold);
    CHECK(state.current_style().style == (fg(blue) | bold));

    state.push_style(fg(red));
    CHECK(state.current_style().style == (fg(red) | bold));

    state.pop_style();
    CHECK(state.current_style().style == fg(blue));

    state.push_style(abs(fg(green)));
    CHECK(state.current_style().style == fg(green));

    state.reset_style();
    CHECK(state.current_style().style == fg(blue));
}

TEST_CASE("styled_out: StyleOutputState: nested style state") {
    using namespace deco;

    TestStyleOutputState state;
    state.set_base_style(fg(blue)).enable_nesting(true);

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

TEST_CASE("styled_out: StyledOstream: write plain values") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);

    double d = 1.5;

    styled_os << "value=" << 42 << ' ' << d;

    CHECK(os.str()
          == abs(null_style).to_escape() + std::string("value=42 1.5"));
}

TEST_CASE("styled_out: StyledOstream: call IO manipulator") {
    using namespace deco;

    std::ostringstream os;
    std::ostringstream expected;
    StyledOstream styled_os(os);

    styled_os << std::boolalpha << true << ' ' << std::noboolalpha << false
              << ' ' << std::showbase << std::hex << 42 << ' '
              << std::noshowbase << std::dec << 42 << ' ' << std::uppercase
              << std::scientific << std::setprecision(3) << 1.5 << ' '
              << std::nouppercase << std::fixed << std::setprecision(2) << 1.5
              << ' ' << std::defaultfloat << std::showpos << 7 << ' '
              << std::noshowpos << std::showpoint << 2.0 << ' '
              << std::noshowpoint << std::setfill('.') << std::left
              << std::setw(5) << 12 << ' ' << std::right << std::setw(5) << 12
              << ' ' << std::internal << std::showpos << std::setw(5) << 12
              << std::noshowpos << ' ' << set_width_4 << 9 << std::endl
              << std::flush << std::ends;

    expected << std::boolalpha << true << ' ' << std::noboolalpha << false
             << ' ' << std::showbase << std::hex << 42 << ' ' << std::noshowbase
             << std::dec << 42 << ' ' << std::uppercase << std::scientific
             << std::setprecision(3) << 1.5 << ' ' << std::nouppercase
             << std::fixed << std::setprecision(2) << 1.5 << ' '
             << std::defaultfloat << std::showpos << 7 << ' ' << std::noshowpos
             << std::showpoint << 2.0 << ' ' << std::noshowpoint
             << std::setfill('.') << std::left << std::setw(5) << 12 << ' '
             << std::right << std::setw(5) << 12 << ' ' << std::internal
             << std::showpos << std::setw(5) << 12 << std::noshowpos << ' '
             << set_width_4 << 9 << std::endl
             << std::flush << std::ends;

    CHECK(os.str() == abs(null_style).to_escape() + expected.str());
}

TEST_CASE("styled_out: StyledOstream: throws if output operator returns different "
          "ostream object") {
    using namespace deco;

    StyledOstream styled_os(std::cout);
    CHECK_THROWS_AS(styled_os << return_dirrerent_ostream_t {},
                    std::logic_error);
}

TEST_CASE("styled_out: StyledOstream: write styles") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);

    styled_os << fg(red) << "error" << reset;

    CHECK(os.str()
          == abs(null_style).to_escape() + fg(red).to_escape()
                 + std::string("error") + abs(null_style).to_escape());
}

TEST_CASE("styled_out: StyledOstream: disable style output") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);
    styled_os.enable_style(false);

    styled_os << fg(red) << "error" << reset;

    CHECK(os.str() == "error");
    CHECK(styled_os.current_style().style == null_style);
}

TEST_CASE("styled_out: StyledOstream: restores nested styles") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os =
        styled_out(os).set_base_style(fg(blue)).enable_nesting(true);

    styled_os << bold << "bold" << fg(red) << "red bold" << pop << "blue bold"
              << pop << "blue";

    CHECK(os.str()
          == abs(fg(blue)).to_escape() + bold.to_escape()
                 + std::string("bold") + fg(red).to_escape()
                 + std::string("red bold")
                 + abs(fg(blue) | bold).to_escape()
                 + std::string("blue bold") + abs(fg(blue)).to_escape()
                 + std::string("blue"));
}

TEST_CASE("styled_out: StyledOstream: reset returns to base style") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os =
        styled_out(os).set_base_style(fg(blue)).enable_nesting(true);

    styled_os << bold << "bold" << reset << "base";

    CHECK(os.str()
          == abs(fg(blue)).to_escape() + bold.to_escape()
                 + std::string("bold") + abs(fg(blue)).to_escape()
                 + std::string("base"));
}

TEST_CASE("styled_out: StyledOstream: context tracking") {
    using namespace deco;

    std::ostringstream os1;
    std::ostringstream os2;
    auto out1 = styled_out(os1).set_base_style(fg(blue));
    auto out2 = styled_out(os2).set_base_style(fg(red));

    out1 << "a" << "b";
    out2 << "c";
    out1 << "d" << styled("e", bold) << "f";

    CHECK(os1.str()
          == abs(fg(blue)).to_escape() + std::string("ab")
                 + abs(fg(blue)).to_escape() + std::string("d")
                 + bold.to_escape() + std::string("e")
                 + abs(fg(blue)).to_escape() + std::string("f"));
    CHECK(os2.str() == abs(fg(red)).to_escape() + std::string("c"));
}

TEST_CASE("styled_out: StyledOstream: only output base style first time if context "
          "tracking is off") {
    using namespace deco;

    std::ostringstream os;
    auto out =
        StyledOstream(os).set_base_style(fg(blue));

    out << "a" << "b" << bold << "c" << pop << "d" << reset << "e";

    CHECK(os.str()
          == abs(fg(blue)).to_escape() + std::string("ab")
                 + bold.to_escape() + std::string("c")
                 + abs(fg(blue)).to_escape() + std::string("d")
                 + abs(fg(blue)).to_escape() + std::string("e"));
}

// NOLINTEND
