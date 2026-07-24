#include <decoterm/decoterm.hpp>

#include <iostream>
#include <string_view>

// clang-format off

auto main() -> int {
    // -- 1. Basic Style output

    deco::Style style = deco::underline | deco::fg(deco::bright_green);
    deco::Style warning_style = deco::bold | deco::fg(deco::rgb(0xfff2b2));

    std::cout   << (deco::bold | deco::fg(deco::bright_blue))
                << "Hello "
                << style
                << "World!"
                << deco::reset      // Reset to default style
                << "\n\n";

    // just like fmt::styled() from fmtlib
    std::cout << deco::styled("warning: ", warning_style)
              << "insert warning message here"
              << deco::styled("[-Wunused-variable]", warning_style | deco::italic)
              << '\n';

    // HSV usage (maybe)
    std::string_view text = "Rainbooowww~~~~~~~";
    std::cout << deco::italic;
    for (std::size_t i = 0; i < text.size(); ++i) {
        std::cout << deco::fg(deco::hsv(i * (360 / text.size()), 120, 255))
                  << text[i];
    }
    std::cout << "\n\n";

    // -- 2. Advanced stateful output

    deco::StyledOstream sout = deco::styled_out(std::cout)
        .base_style(deco::bg(deco::rgb(32, 32, 32)))
        .enable_nesting();

    // Style nesting
    sout    << "base style "
                << deco::fg(deco::bright_blue) << "{ blue "
                    << deco::italic << "{ italic blue "
                        << deco::fg(deco::bright_red) << "{ italic red } "
                    << deco::pop << "italic blue } "
                << deco::pop << "blue } "
            << deco::pop << "base style";

    
}
