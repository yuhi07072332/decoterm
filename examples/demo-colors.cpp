#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <array>
#include <iostream>
#include <string_view>

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

void print_color_cell(int index, bool is_fg_white) {
    const auto bg_color = deco::Color(index);
    const auto fg_color = is_fg_white ? deco::white : deco::black;

    std::print("{}{:^4}{}", color(fg_color, bg_color), index, deco::reset);
}

auto main() -> int {
    using namespace deco;
    constexpr Style h1 = color(black, bright_green) | bold;
    constexpr Style h2 = underline | bold;

    std::cout << h1 % "Terminal Colors" << "\n\n";

    std::cout << h2 % "ANSI 16 colors:\n" << '\n';

    for (int i = 0; i < 8; ++i) {
        const Color fg_color = i == 0 ? white : black;
        std::println("[{:0>2}]{:15}{:15}   [{:0>2}]{:15}{:15}",
                     i,
                     fg(i) % color_names[i],
                     color(fg_color, i) % color_names[i],
                     i + 8,
                     fg(i + 8) % color_names[i + 8],
                     color(fg_color, i + 8) % color_names[i + 8]);
    }

    std::cout << '\n' << h2 % "XTerm 256 colors:\n" << '\n';

    std::cout << bold % "6 x 6 x 6 color cube [16, 231]:\n";

    for (int i = 0; i < 2; ++i) {
        for (int row = 0; row < 6; ++row) {
            for (int surface = i * 3; surface < (i * 3) + 3; ++surface) {
                for (int col = 0; col < 6; ++col) {
                    int abs_col = 16 + (surface * 6) + col;
                    print_color_cell((row * 36) + abs_col, abs_col < 34);
                }
                std::print("  ");
            }
            std::println();
        }
        std::println();
    }

    std::cout << bold % "greyscale [232, 255]:\n";
    for (int i = 232; i <= 255; ++i) {
        print_color_cell(i, i <= 243);
    }

    std::cout << "\n\n" << h1 % "True color" << "\n\n";

    std::cout << "R: ";
    for (int i = 0; i < 255; i += 3)
        std::cout << bg(rgb(i, 0, 0)) << ' ';
    std::cout << reset << '\n';

    std::cout << "G: ";
    for (int i = 0; i < 255; i += 3)
        std::cout << bg(rgb(0, i, 0)) << ' ';
    std::cout << reset << '\n';

    std::cout << "B: ";
    for (int i = 0; i < 255; i += 3)
        std::cout << bg(rgb(0, 0, i)) << ' ';
    std::cout << reset << '\n';

    std::cout << "C: ";
    for (int i = 0; i < 255; i += 3)
        std::cout << bg(rgb(i, i, 0)) << ' ';
    std::cout << reset << '\n';

    std::cout << "M: ";
    for (int i = 0; i < 255; i += 3)
        std::cout << bg(rgb(i, 0, i)) << ' ';
    std::cout << reset << '\n';

    std::cout << "Y: ";
    for (int i = 0; i < 255; i += 3)
        std::cout << bg(rgb(0, i, i)) << ' ';
    std::cout << reset << '\n';

    std::cout << "\n";
    for (int v = 0; v < 256; v += 16) {
        for (int h = 0; h < 360; h += 4) {
            std::print("{}▀", color(hsv(h, 255, v), hsv(h, 255, v + 8)));
        }
        std::println("{}", reset);
    }
}
