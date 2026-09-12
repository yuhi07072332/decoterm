#include <decoterm/decoterm.hpp>

#include <iostream>

auto main() -> int {
    // combine `Style`s with `|`
    const deco::Style error =
        deco::bold | deco::invert | deco::fg(deco::bright_red);

    std::cout << error << "assertion failed " << deco::reset << "\nat ";

    // same as `std::cout << deco::italic << "main.cpp" << deco::reset`
    std::cout << deco::italic % "main.cpp";

    // %` is a shorthand for `styled(style, value)`.
    // styled` can also group multiple values and contain nested styles.
    std::cout << ":" << deco::bold % 116 << ": "
              << deco::styled(deco::fg(deco::rgb(238, 212, 159)),
                              "expected ",
                              deco::italic % 3.14)
              << ", "
              << deco::styled(deco::fg(deco::bright_magenta),
                              "got ",
                              deco::bold % 2.71)
              << "\n\n";
}
