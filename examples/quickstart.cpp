#include <decoterm/decoterm.hpp>

#include <iostream>

// clang-format off

auto main() -> int {
    // combine `Style`s with `|`

    const deco::Style error_style = 
        deco::bold | deco::invert | deco::fg(deco::bright_red);
    const deco::Style expected_style = 
        deco::bold | deco::fg(deco::rgb(238, 212, 159));

    // use `Style`s like I/O manipulators

    std::cout << error_style << "assertion failed " << deco::reset
              << "at ";

    // use `Style`s as a function to decorate values

    // -- same as `std::cout << deco::italic << "main.cpp" << deco::reset`
    std::cout << deco::italic("main.cpp");

    // -- this can also store multiple values.
    std::cout << ":" << deco::bold(116) << ":   "
              << expected_style("expected ", deco::underline(3.14))
              << ", "
              << fg(deco::bright_magenta)("got ", deco::underline(2.71))
              << "\n\n";

    // StyledOstream: ostream wrapper with style output states.

    auto out = deco::StyledOstream(std::cout);

    out.set_base_style(deco::bg(deco::rgb(32, 32, 32)))
       .enable_style(deco::terminal::is_stdout_tty())
       .enable_nesting(true);
}
