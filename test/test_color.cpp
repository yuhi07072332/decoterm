#include <decoterm/decoterm.hpp>

#include <array>
#include <iostream>
#include <print>
#include <string_view>

inline constexpr std::array<std::string_view, 16> color_names {
    "Black",
    "Red",
    "Green",
    "Yellow",
    "Blue",
    "Magenta",
    "Cyan",
    "White",
    "BlackLight",
    "RedLight",
    "GreenLight",
    "YellowLight",
    "BlueLight",
    "MagentaLight",
    "CyanLight",
    "WhiteLight",
};

void print_color_cell(int color_index) {
    const auto background = deco::Color(static_cast<deco::Colors>(color_index));
    const auto foreground = color_index <= 213 ? deco::white : deco::black;

    std::print("{}{:<3}{}", deco::color(foreground, background), color_index, deco::reset);
}

int main() {
    using namespace deco;

    std::println("{}========= ANSI Colors ========={}", fg(yellow), reset);

    std::println("system colors [0, 15]:\n");
    for (int i = 0; i <= 7; ++i) {
        auto col = Color(static_cast<Colors>(i));
        auto colorlight = Color(static_cast<Colors>(i + 8));
        auto color_fg = col == black ? white : black;

        std::println(
            "{2}{0:<12} {3}{0:<12} {4}{1:<12} {5}{1:<12}{6}",
            color_names[i],
            color_names[i + 8],
            fg(col),
            color(color_fg, col),
            color(colorlight, Color::Default),
            color(color_fg, colorlight),
            reset
        );
    }

    std::println("\n6 x 6 x 6 color cube [16, 231]:\n");
    for (int cube_pair = 0; cube_pair < 6; cube_pair += 2) {
        for (int row = 0; row < 6; ++row) {
            for (int cube = cube_pair; cube < cube_pair + 2; ++cube) {
                for (int col = 0; col < 6; ++col) {
                    print_color_cell(16 + cube * 36 + row * 6 + col);
                    std::print(" ");
                }
                std::print("  ");
            }
            std::println();
        }
        std::println();
    }

    std::println("grayscale colors [232, 255]:\n");
    for (int color_index = 232; color_index <= 255; ++color_index) {
        print_color_cell(color_index);
    }
    std::println();
}
