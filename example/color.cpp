#include <decoterm/style.hpp>
#include <decoterm/format.hpp>

#include <iostream>

constexpr std::array<std::string_view, 16> color_names {
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "magenta",
    "cyan",
    "white",
    "bright_black",
    "bright_red",
    "bright_green",
    "bright_yellow",
    "bright_blue",
    "bright_magenta",
    "bright_cyan",
    "bright_white",
};

auto main() -> int {
    using namespace deco;
    constexpr Style h1 = color(black, bright_green) | bold;
    constexpr Style h2 = underline | bold;

    std::cout << styled("Terminal Colors\n", h1) << '\n';

    std::cout << styled("ANSI 16 colors:\n", h2) << '\n';

    for (int i = 0; i < 16; ++i) {
        std::println("{:12}   {:12}", styled(color_names[i], fg(i)), styled(color_names[i + 8], fg(i + 8)));
    }

    std::cout << styled("XTerm 256 colors:\n", h2) << '\n';
    
}
