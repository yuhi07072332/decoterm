#include <decoterm/style.hpp>
#include <decoterm/output.hpp>

#include "unit_test.hpp"
#include <doctest.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

struct ReturnDifferentOstream {};

auto operator<<(std::ostream& os, ReturnDifferentOstream) -> std::ostream& {
    return std::cerr;
}

auto set_width_4(std::basic_ios<char>& ios) -> std::basic_ios<char>& {
    ios.width(4);
    return ios;
}

const deco::OutputConfig ocfg;

auto pending_escape(const deco::detail::StyleContext& ctx) -> EscapeSeq {
    return ctx.pending().visit(
        [](deco::detail::style_type auto style) { return style.to_escape(); });
}

} // namespace

// NOLINTBEGIN

using namespace deco;
using doctest::test_suite;

TEST_CASE("Buffer opeartions" * test_suite("Buffer")) {
    detail::Buffer<char, 3> buf;

    SUBCASE("push and pop") {
        CHECK(buf.empty());

        buf.push_back('a');
        buf.push_back('b');
        CHECK_FALSE(buf.empty());
        CHECK(buf.back() == 'b');
        CHECK(buf.size() == 2);

        buf.pop_back();
        CHECK(buf.back() == 'a');
        buf.pop_back();
        CHECK(buf.empty());
    }

    SUBCASE("pop empty buffer") {
        buf.pop_back();
        CHECK(buf.empty());
    }

    SUBCASE("clear") {
        buf.push_back('a');
        buf.push_back('b');
        buf.clear();
        CHECK(buf.empty());

        buf.push_back('a');
        CHECK(buf.back() == 'a');
        buf.pop_back();
        CHECK(buf.empty());
    }
}

TEST_CASE("Buffer grows to heap" * test_suite("StyleStack")) {
    detail::Buffer<char, 2> buf;

    SUBCASE("grow once") {
        for (char c : std::string_view("abc"))
            buf.push_back(c);
        CHECK(buf.back() == 'c');

        buf.pop_back();
        CHECK(buf.back() == 'b');
        buf.pop_back();
        CHECK(buf.back() == 'a');
        buf.pop_back();
        CHECK(buf.empty());
    }
    SUBCASE("grow more than once") {
        for (char c : "hello, world")
            buf.push_back(c);

        CHECK(std::string_view(buf.data()) == "hello, world");
    }
}

TEST_CASE("Buffer copy and move" * test_suite("StyleStack")) {
    detail::Buffer<AbsoluteStyle, 3> local;
    local.push_back(abs(bold));
    local.push_back(abs(italic));

    detail::Buffer<AbsoluteStyle, 2> heap;
    heap.push_back(abs(bold));
    heap.push_back(abs(italic));
    heap.push_back(abs(underline));

    SUBCASE("copy") {
        const auto local_copy = local;
        local.back() = abs(underline);
        CHECK(local_copy.back() == abs(italic));

        const auto heap_copy = heap;
        heap.back() = abs(blink);
        CHECK(heap_copy.back() == abs(underline));
    }

    SUBCASE("move") {
        auto moved_local = std::move(local);
        CHECK(moved_local.back() == abs(italic));
        CHECK(local.empty());
        local.push_back(abs(underline));
        CHECK(local.back() == abs(underline));

        auto moved_heap = std::move(heap);
        CHECK(moved_heap.back() == abs(underline));
        CHECK(heap.empty());
        heap.push_back(abs(blink));
        CHECK(heap.back() == abs(blink));
    }
}

TEST_CASE("StyleContext manages pending style" * test_suite("StyleContext")) {
    detail::StyleContext ctx;
    ctx.set_base_style(bold);
    CHECK(pending_escape(ctx) == to_escape(abs(bold)));

    static_cast<void>(ctx.consume_pending());

    SUBCASE("push()") {
        ctx.push(italic);
        ctx.push(fg(blue));
        CHECK(ctx.current_abstyle() == abs(bold | italic | fg(blue)));
        CHECK(pending_escape(ctx) == to_escape(italic | fg(blue)));
    }

    SUBCASE("pop() restores previous style") {
        ctx.push(abs(italic));
        ctx.push(fg(red));
        CHECK(pending_escape(ctx) == to_escape(abs(italic | fg(red))));

        ctx.pop();
        CHECK(ctx.current_abstyle() == abs(italic));
        CHECK(pending_escape(ctx) == to_escape(abs(italic)));

        ctx.pop();
        CHECK(ctx.current_abstyle() == abs(bold));
        CHECK(pending_escape(ctx) == to_escape(abs(bold)));
    }

    SUBCASE("reset() merges base style") {
        ctx.push(italic);
        CHECK(pending_escape(ctx) == to_escape(italic));

        ctx.reset();
        CHECK(ctx.current_abstyle() == abs(bold));
        CHECK(pending_escape(ctx) == to_escape(abs(bold)));
    }

    SUBCASE("ensure_style()") {
        ctx.push(italic);

        ctx.ensure_style();
        CHECK(pending_escape(ctx) == to_escape(abs(bold | italic)));
    }

    SUBCASE("set_current_style()") {
        ctx.set_current_style(italic);
        CHECK(ctx.current_abstyle() == abs(bold | italic));
        CHECK(pending_escape(ctx) == to_escape(italic));

        ctx.set_current_style(abs(fg(red)));
        CHECK(ctx.current_abstyle() == abs(fg(red)));
        CHECK(pending_escape(ctx) == to_escape(abs(fg(red))));
    }
}

TEST_CASE("StyleContext manages base style" * test_suite("StyleContext")) {
    detail::StyleContext ctx;
    ctx.set_base_style(bold);

    SUBCASE("merges when stack is empty") {
        CHECK(ctx.base_style() == bold);
        CHECK(ctx.current_abstyle() == abs(bold));
        CHECK(pending_escape(ctx) == to_escape(abs(bold)));
    }

    SUBCASE("don't merge when stack is not empty") {
        ctx.push(italic);

        static_cast<void>(ctx.consume_pending());

        ctx.set_base_style(fg(red));
        CHECK(ctx.base_style() == fg(red));
        CHECK(ctx.current_abstyle() == abs(bold | italic));
        CHECK_FALSE(ctx.has_pending());
    }
}

TEST_CASE("Ostream writes values" * test_suite("Ostream")) {
    std::ostringstream os;
    std::ostringstream expected;

    Ostream out(ocfg, os);

    SUBCASE("plain values") {
        out << "answer=" << 42 << ' ' << true;
        expected << "answer=" << 42 << ' ' << true;
        CHECK(os.str() == expected.str());
    }

    SUBCASE("IO manipulators") {
        out.ensure_style();
        out << std::boolalpha << true << ' ' << std::noboolalpha << false << ' '
            << std::showbase << std::hex << 42 << ' ' << std::noshowbase
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
        CHECK(os.str() == expected.str());
    }

}

TEST_CASE("Ostream throws error if output operator returns different std::ostream" * test_suite("Ostream")) {
    std::ostringstream os;
    Ostream out(ocfg, os);
    CHECK_THROWS_AS(out << ReturnDifferentOstream {}, std::logic_error);
}

TEST_CASE("Ostream writes styled" * test_suite("Ostream")) {
    std::ostringstream os;
    std::ostringstream expected;
    Ostream out(ocfg, os);

    out.set_base_style(bold);
    out << "before" << styled("inside", italic) << "after";
    expected << abs(bold) << "before" << italic << "inside" << abs(bold)
             << "after";

    CHECK(os.str() == expected.str());
}

TEST_CASE("Ostream merges styles" * test_suite("Ostream")) {
    std::ostringstream os;
    std::ostringstream expected;
    Ostream out(ocfg, os);

    SUBCASE("merges consecutive styles and pushes") {
        out << bold << fg(red) << "one" << push(underline) << bg(blue) << "two";
        expected << (bold | fg(red)) << "one" << (underline | bg(blue))
                 << "two";

        CHECK(os.str() == expected.str());
    }

    SUBCASE("merges pending style with styled value") {
        out << bold << styled("text", italic);
        expected << (bold | italic) << "text" << abs(bold);

        CHECK(os.str() == expected.str());
    }
}

TEST_CASE("Ostream tracks current style" * test_suite("Ostream")) {
    std::ostringstream os;
    Ostream out(ocfg, os);

    out.set_base_style(bold);
    CHECK(out.base_style() == bold);
    CHECK(out.current_style() == bold);

    out << push(italic);
    CHECK(out.current_style() == (bold | italic));

    out << push(fg(red));
    CHECK(out.current_style() == (bold | italic | fg(red)));

    out << pop;
    CHECK(out.current_style() == (bold | italic));

    out << reset;
    CHECK(out.current_style() == bold);
}

// NOLINTEND
