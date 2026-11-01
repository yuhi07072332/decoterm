#include <decoterm/deco.hpp>
#include <decoterm/color256.hpp>

#include <print>
#include <iostream>

[[clang::suppress("unused-variable")]]
auto main() -> int {
    // [defining-color]

    namespace colors = deco::colors;

    // system color
    constexpr deco::Color info_color = colors::bright_cyan;

    // xterm 256 color (requires color256.hpp)
    constexpr deco::Color xterm_256_color = colors::dark_violet;

    // from rgb
    constexpr deco::Color error_color = colors::rgb(237, 135, 150);

    // from hsv
    const deco::Color mauve = colors::hsv(267, 211, 204);

    // [/defining-color]

    // [defining-style]

    namespace styles = deco::styles;

    // use `|` to combine styles
    constexpr deco::Style error_style = styles::bold 
        | styles::underline
        | styles::fg(colors::black)
        | styles::bg(error_color);

    constexpr deco::Style info_style = styles::bold | styles::fg(info_color);

    // [/defining-style]

    // [output-style]

    std::cout << error_style << "ERROR" << deco::reset << " blahblahblah\n";
    std::println("The answer is {}42{}.", info_style, deco::reset);

    // [/output-style]

    // [styled]

    std::cout << deco::styled("ERROR", error_style) << " blahblahblah\n";
    std::println("The answer is {}.", 42 | info_style);

    // [/styled]
}
