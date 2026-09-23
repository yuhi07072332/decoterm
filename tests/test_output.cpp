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

struct return_different_ostream_t {};

auto operator<<(std::ostream& os, return_different_ostream_t)
    -> std::ostream& {
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

// TODO: test cases for detail::StyleStack

TEST_CASE("StyleState options" * test_suite("StyleState")) {
}

TEST_CASE("StyleState copy and move" * test_suite("StyleState")) {
    SUBCASE("copy constructs from state") {
    }

    SUBCASE("move constructs from state") {
    }

    SUBCASE("copy assignment clears stale context tracking") {
    }
}

TEST_CASE("StyledOstream writes values" * test_suite("StyledOstream")) {
    std::ostringstream os;
    std::ostringstream expected;

    SUBCASE("plain values") {
    }

    SUBCASE("IO manipulators") {
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
}

TEST_CASE("StyledOstream writes styles" * test_suite("StyledOstream")) {
    // TODO: test pending style works
}

TEST_CASE("StyledOstream tracks current style" * test_suite("StyledOstream")) {
    // TODO: test pending style works
    SUBCASE("nested styles restore previous style") {
    }

    SUBCASE("reset returns to base style") {
    }

    SUBCASE("context tracking keeps independent streams") {
    }
}

TEST_CASE("StyledOstream writes Styled" * test_suite("StyledOstream")) {
}

// NOLINTEND
