#define DECOTERM_NO_FORMAT
#include <decoterm/sync.hpp>

#include "unit_test.hpp"
#include <doctest.h>
#include <sstream>

// NOLINTBEGIN

using namespace deco;
using doctest::test_suite;

TEST_CASE("OSyncStream" * test_suite("OSyncStream")) {
    std::ostringstream os;
    std::ostringstream expected;

    expected << reset << "text" << bold << 42 << fg(blue) << std::endl
        << italic << "text" << abs(bold | fg(blue)) << 42;

    SUBCASE("emit manually") {
        OSyncStream out(os);
        out << "text" << bold << 42 << fg(blue) << std::endl
            << push(italic) << "text" << pop << 42;

        CHECK(os.str().empty());

        out.emit();
        CHECK(os.str() == expected.str());
    }

    SUBCASE("emits on destruct") {
        {
            OSyncStream out(os);
            out << "text" << bold << 42 << fg(blue) << std::endl
                << push(italic) << "text" << pop << 42;
            CHECK(os.str().empty());
        }
        CHECK(os.str() == expected.str());
    }
}

// NOLINTEND
