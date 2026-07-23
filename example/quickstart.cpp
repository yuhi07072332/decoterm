#include <decoterm/decoterm.hpp>

#include <iostream>
#include <string_view>

auto main() -> int {
    // -- 1. Basic Style output

    deco::Style style = deco::underline | deco::fg(deco::bright_green);
    deco::Style warning_style = deco::bold | deco::fg(deco::rgb(0xfbfcc5));

    std::cout   << (deco::bold | deco::fg(deco::bright_blue))
                << "Hello "
                << style
                << "World!"
                << deco::reset      // Reset to default style
                << '\n';

    // just like fmt::styled() from fmtlib
    std::cout << deco::styled("warning: ", warning_style)
              << "insert warning message here"
              << deco::styled("[-Wunused-variable]", warning_style | deco::italic)
              << '\n';

    // HSV usage (idk)
    std::string_view text = "Rainbooowww~~~~~~~";
    std::cout << deco::italic;
    for (std::size_t i = 0; i < text.size(); ++i) {
        std::cout << deco::fg(deco::hsv(i * (360 / text.size()), 120, 255))
                  << text[i];
    }
    std::cout << "\n\n";

    // -- 2. Advanced

    deco::StyledOstream styled_out = deco::styled_out(std::cout)
        .base_style(deco::bg(deco::rgb(64, 64, 64)))
        .enable_style(deco::terminal::is_stdout_tty())
        .enable_nesting();

}
