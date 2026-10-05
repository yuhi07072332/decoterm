#include "decoterm/output.hpp"
#include <decoterm/decoterm.hpp>

#include <iostream>

auto main() -> int {
    // combine `Style`s with `|`
    const deco::Style error =
        deco::bold | deco::invert | deco::fg(deco::bright_red);

    std::cout << error << "assertion failed " << deco::reset << "\nat ";

    // same as `std::cout << deco::italic << "main.cpp" << deco::reset`
    std::cout << (deco::italic | "main.cpp");
}
