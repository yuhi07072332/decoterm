#include <decoterm/decoterm.hpp>
#include <decoterm/format.hpp>
#include <chrono>
#include <iostream>
#include <print>
#include <string_view>
#include <thread>

using namespace deco;

constexpr long TIMES = 100000;
constexpr std::string_view ENABLE_ALTERNATE_SCREEN = "\x1b[?1049h";
constexpr std::string_view DISABLE_ALTERNATE_SCREEN = "\x1b[?1049l";

auto measure(auto&& fn) -> double {
    using namespace std::chrono_literals;
    auto begin = std::chrono::steady_clock::now();
    fn();
    auto end = std::chrono::steady_clock::now();
    return (end - begin) / 1ns / 1000000.0;     //NOLINT
}

auto main() -> int {    // NOLINT
    using namespace std::chrono_literals;

    std::print(ENABLE_ALTERNATE_SCREEN);
    std::this_thread::sleep_for(10ms);

    measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::cout << "\x1b[34m\x1b[m";
        }
    });

    // SECTION1

    auto os_raw = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::cout << "\x1b[34m\x1b[m";
        }
    });

    auto os_style = measure([]{
        constexpr Style style = fg(blue);
        for (int i = 0; i < TIMES; ++i) {
            std::cout << style << deco::reset;
        }
    });

    auto fmt_raw = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::print("\x1b[34m\x1b[m");
        }
    });

    auto fmt_style = measure([]{
        constexpr Style style = fg(blue);
        for (int i = 0; i < TIMES; ++i) {
            std::print("{}{}", style, reset);
        }
    });

    // SECTION2
    auto os_raw_2 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::cout << "\x1b[48;2;156;234;108m\x1b[;m";
        }
    });

    auto os_style_2 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::cout << bg(rgb(156, 234, 108)) << reset;
        }
    });

    auto fmt_raw_2 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::print("\x1b[48;2;156;234;108m\x1b[;m");
        }
    });

    auto fmt_style_2 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::print("{}{}", bg(rgb(156, 234, 108)), reset);
        }
    });

    // SECTION3
    auto os_raw_3 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::cout << "\x1b[48;2;"
                << (i&255) << ';'
                << ((i>>8)&255) << ';'
                << ((i>>16)&255)
                << "m\x1b[;m";
        }
    });

    auto os_style_3 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::cout << bg(rgb(i&255, (i>>8)&255, (i>>16)&255))
                      << reset;
        }
    });

    auto fmt_raw_3 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::print(
                "\x1b[48;2;{};{};{}m\x1b[;m",
                i&255,
                (i>>8)&225,
                (i>>16)&255
            );
        }
    });

    auto fmt_style_3 = measure([]{
        for (int i = 0; i < TIMES; ++i) {
            std::print("{}{}", bg(rgb(i&255, (i>>8)&255, (i>>16)&255)), reset);
        }
    });

    std::print(DISABLE_ALTERNATE_SCREEN);

    std::println("n = {}", TIMES);

    std::println("{}[SECTION1: basic system color output]{}", fg(yellow), reset);
    std::println("ostream raw escape:      {}ms", os_raw);
    std::println("ostream Style output:    {}ms", os_style);
    std::println("formatter raw escape:    {}ms", fmt_raw);
    std::println("formatter Style output:  {}ms", fmt_style);
    std::println();

    std::println("{}[SECTION2: fixed RGB color output]{}", fg(yellow), reset);
    std::println("ostream raw escape:      {}ms", os_raw_2);
    std::println("ostream Style output:    {}ms", os_style_2);
    std::println("formatter raw escape:    {}ms", fmt_raw_2);
    std::println("formatter Style output:  {}ms", fmt_style_2);
    std::println();

    std::println("{}[SECTION3: dynamic RGB color output]{}", fg(yellow), reset);
    std::println("ostream raw escape:      {}ms", os_raw_3);
    std::println("ostream Style output:    {}ms", os_style_3);
    std::println("formatter raw escape:    {}ms", fmt_raw_3);
    std::println("formatter Style output:  {}ms", fmt_style_3);
    std::println();

    return 0;
}
