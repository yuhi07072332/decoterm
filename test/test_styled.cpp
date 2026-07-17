#include <decoterm/style.hpp>
#include <decoterm/styled.hpp>
#include <doctest.h>

#include <sstream>
#include <string>
#include <type_traits>

namespace {

struct TestStyleOutputState : deco::StyleOutputState {
    using deco::StyleOutputState::pop_style;
    using deco::StyleOutputState::push_style;
    using deco::StyleOutputState::reset_style;
};

struct Counter {
    int value = 0;
};

auto operator<<(std::ostream& os, Counter& counter) -> std::ostream& {
    ++counter.value;
    os << counter.value;
    return os;
}

struct Value {
    int value = 128;
};

auto operator<<(std::ostream& os, const Value& value) -> std::ostream& {
    os << value.value;
    return os;
}

} // namespace

TEST_CASE("styled: styled(): own rvalues") {
    using namespace deco;

    auto styled_value = styled(45, bold);
    const auto const_styled_value = styled(100, bold);

    static_assert(std::is_same_v<decltype(styled_value.get()), int&>);
    static_assert(
        std::is_same_v<decltype(const_styled_value.get()), const int&>);

    styled_value.get() = 128;

    CHECK(styled_value.get() == 128);
    CHECK(const_styled_value.get() == 100);
    CHECK(styled_value.style() == bold);
}

TEST_CASE("styled: styled(): reference lvalues") {
    using namespace deco;

    int value = 30;
    const int const_value = 45;

    auto styled_value = styled(value, bold);
    auto styled_const_value = styled(const_value, bold);
    const auto const_styled_value = styled(value, bold);
    const auto const_styled_const_value = styled(const_value, bold);

    static_assert(std::is_same_v<decltype(styled_value.get()), int&>);
    static_assert(
        std::is_same_v<decltype(styled_const_value.get()), const int&>);
    static_assert(
        std::is_same_v<decltype(const_styled_value.get()), int&>);
    static_assert(std::is_same_v<decltype(const_styled_const_value.get()),
                                 const int&>);

    styled_value.get() = 256;
    CHECK(value == 256);

    const_styled_value.get() = 512;
    CHECK(value == 512);
    CHECK(styled_const_value.get() == 45);
    CHECK(const_styled_const_value.get() == 45);
}

TEST_CASE("styled: styled(): output owning and referenced values") {
    using namespace deco;

    std::ostringstream os;
    const auto const_owned = styled(Value {64}, bold);
    Counter counter;
    const auto const_reference = styled(counter, bold);

    os << styled(Value {32}, bold) << ' ' << const_owned << ' '
       << const_reference;

    CHECK(os.str()
          == bold.to_escape() + std::string("32")
                 + absolute(default_style).to_escape() + std::string(" ")
                 + bold.to_escape() + std::string("64")
                 + absolute(default_style).to_escape() + std::string(" ")
                 + bold.to_escape() + std::string("1")
                 + absolute(default_style).to_escape());
    CHECK(counter.value == 1);
}

TEST_CASE("styled: styled(): output mutable temporaries") {
    using namespace deco;

    std::ostringstream os;

    os << styled(Counter {}, bold);

    CHECK(os.str()
          == bold.to_escape() + std::string("1")
                 + absolute(default_style).to_escape());
}

TEST_CASE("styled: StyleOutputState: options") {
    using namespace deco;

    StyleOutputState state;

    CHECK(state.style_enabled());
    CHECK_FALSE(state.color_fallback_enabled());
    CHECK_FALSE(state.nesting_enabled());
    CHECK(state.base_style() == default_style);
    CHECK(state.current_style().style == default_style);

    state.enable_style(false).enable_color_fallback(true).base_style(fg(blue));

    CHECK_FALSE(state.style_enabled());
    CHECK(state.color_fallback_enabled());
    CHECK(state.base_style() == fg(blue));
    CHECK(state.current_style().style == fg(blue));

    state.enable_nesting(true);
    CHECK(state.nesting_enabled());
    CHECK(state.current_style().style == fg(blue));

    state.enable_nesting(false);
    CHECK_FALSE(state.nesting_enabled());
    CHECK(state.current_style().style == fg(blue));
}

TEST_CASE("styled: StyleOutputState: single style state") {
    using namespace deco;

    TestStyleOutputState state;
    state.base_style(fg(blue));

    state.push_style(bold);
    CHECK(state.current_style().style == (fg(blue) | bold));

    state.push_style(fg(red));
    CHECK(state.current_style().style == (fg(red) | bold));

    state.pop_style();
    CHECK(state.current_style().style == fg(blue));

    state.push_style(absolute(fg(green)));
    CHECK(state.current_style().style == fg(green));

    state.reset_style();
    CHECK(state.current_style().style == fg(blue));
}

TEST_CASE("styled: StyleOutputState: nested style state") {
    using namespace deco;

    TestStyleOutputState state;
    state.base_style(fg(blue)).enable_nesting(true);

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


TEST_CASE("styled: StyledOstream: write plain values") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);

    double d = 1.5;

    styled_os << "value=" << 42 << ' ' << d;

    CHECK(os.str()
          == absolute(default_style).to_escape() + std::string("value=42 1.5"));
}

TEST_CASE("styled: StyledOstream: write styles") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);

    styled_os << fg(red) << "error" << reset;

    CHECK(os.str()
          == absolute(default_style).to_escape() + fg(red).to_escape()
                 + std::string("error") + absolute(default_style).to_escape());
}

TEST_CASE("styled: StyledOstream: disable style output") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);
    styled_os.enable_style(false);

    styled_os << fg(red) << "error" << reset;

    CHECK(os.str() == "error");
    CHECK(styled_os.current_style().style == default_style);
}

TEST_CASE("styled: StyledOstream: restores nested styles") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);
    styled_os.base_style(fg(blue)).enable_nesting(true);

    styled_os << bold << "bold" << fg(red) << "red bold" << pop << "blue bold"
              << pop << "blue";

    CHECK(os.str()
          == absolute(fg(blue)).to_escape() + bold.to_escape()
                 + std::string("bold") + fg(red).to_escape()
                 + std::string("red bold")
                 + absolute(fg(blue) | bold).to_escape()
                 + std::string("blue bold") + absolute(fg(blue)).to_escape()
                 + std::string("blue"));
}

TEST_CASE("styled: StyledOstream: reset returns to base style") {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os(os);
    styled_os.base_style(fg(blue)).enable_nesting(true);

    styled_os << bold << "bold" << reset << "base";

    CHECK(os.str()
          == absolute(fg(blue)).to_escape() + bold.to_escape()
                 + std::string("bold") + absolute(fg(blue)).to_escape()
                 + std::string("base"));
}

// TEST_CASE("styled: StyledOstream: output same style twice ") {
//     using namespace deco;
//
//     std::ostringstream os;
//     StyledOstream styled_os(os);
//
//     styled_os << bold << bold;
//     CHECK(os.str() == bold.to_escape());
//
// }
