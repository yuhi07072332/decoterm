#include "decoterm/styled_out.hpp"
#include <decoterm/decoterm.hpp>
#include <decoterm/format.hpp>
#include <decoterm/terminal.hpp>

#include <iostream>
#include <print>
#include <string_view>

using namespace deco;

constexpr std::array<std::string_view, 16> color_names {
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

constexpr std::array<std::string_view, 8> style_names {
    "Bold",
    "Dim",
    "Italic",
    "Underline",
    "Blink",
    "Invert",
    "Strikethrough",
    "UnderlineDouble",
};

void print_color_cell(int color_index, bool is_fg_white) {
    const auto background = Color(color_index);
    const auto foreground = is_fg_white ? white : black;

    std::print("{}{:^4}{}", color(foreground, background), color_index, reset);
}

auto main() -> int {
    Style style_h1 = color(bright_yellow, rgb(0x1f1e55)) | bold;
    Style style_h2 = fg(bright_yellow) | underline;

    std::cout << styled("SECTION1: Color output", style_h1) << "\n\n";
    std::cout << styled("[Ansi Colors]", style_h2) << "\n\n";

    std::println("system colors [0, 15]:\n");
    for (int i = 0; i <= 7; ++i) {
        auto col = Color(i);
        auto colorlight = Color(i);
        auto color_fg = col == black ? white : black;

        std::println("{2}{0:<12} {3}{0:<12}{6}  {4}{1:<12} {5}{1:<12}{6}",
                     color_names[i],
                     color_names[i + 8],
                     fg(col),
                     color(color_fg, col),
                     color(colorlight, default_color),
                     color(color_fg, colorlight),
                     reset);
    }

    std::println();

    std::println("6 x 6 x 6 color cube [16, 231]:\n");
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

    std::println();

    std::println("grayscale colors [232, 255]:\n");
    for (int color_index = 232; color_index <= 255; ++color_index) {
        print_color_cell(color_index, color_index <= 243);
    }
    std::println("{}", reset);

    std::println();

    std::cout << styled("[True Color]", style_h2) << "\n\n";

    constexpr int step = 3;
    for (int r = 0; r <= 255; r += step)
        std::print("{} ", bg(rgb(r, 0, 0)));
    std::println("{}", reset);

    for (int g = 0; g <= 255; g += step)
        std::print("{} ", bg(rgb(0, g, 0)));
    std::println("{}", reset);

    for (int b = 0; b <= 255; b += step)
        std::print("{} ", bg(rgb(0, 0, b)));
    std::println("{}", reset);

    for (int y = 0; y <= 255; y += step)
        std::print("{} ", bg(rgb(y, y, 0)));
    std::println("{}", reset);

    for (int c = 0; c <= 255; c += step)
        std::print("{} ", bg(rgb(0, c, c)));
    std::println("{}", reset);

    for (int m = 0; m <= 255; m += step)
        std::print("{} ", bg(rgb(m, 0, m)));
    std::println("{}", reset);

    std::println();

    for (int v = 0; v < 256; v += 16) {
        for (int h = 0; h < 360; h += 4) {
            std::print("{}▀", color(hsv(h, 255, v), hsv(h, 255, v + 8)));
        }
        std::println("{}", reset);
    }

    std::cout << styled("\nSECTION2: Style output", style_h1) << "\n\n";

    for (int i = 0; i < style_names.size(); ++i) {
        std::print("{}{}{}  ", Style(1 << i), style_names[i], reset);

        std::println();
    }

    std::cout << styled("\nSECTION3: StyledOutputState", style_h1) << "\n\n";

    auto dout = styled_ostream(std::cout).enable_context();

    dout << styled("[Basic]\n\n", style_h2);

    auto out = styled_ostream(std::cout).enable_context().set_base_style(color(white, rgb(0x21314d))).enable_nesting();
    out << "base style "
        << styled("{ styled: italic fg(yellowlight) }", italic | fg(bright_yellow))
        << " base style "
        << absolute(null_style) << "{ absolute(default_style) }" << reset
        << " base style";

    out << absolute(null_style) << "\n\n" 
        << style_h2 << "[Style nesting]\n\n" << reset;

    out.enable_nesting(true);
    out << "base style "
        << (italic | fg(bright_red)) << "{ italic | fg(bright_red) "
        << (underline | color(bright_yellow, default_color)) << "{ underline | color(bright_yellow, default_color) }"
        << pop << " pop }" << pop << " base style";

    dout << styled("\n\n[Styled Output context]\n\n", style_h2);

    auto out1 = styled_ostream(std::cout).enable_context(true);
    out1 << (italic | fg(bright_green));

    auto out2 = styled_ostream(std::cout).enable_context(true);

    out1 << "{ out1 }";
    out2 << "{ out2 }";
    out1 << "{ out1 }";

    dout << styled("\n\n[Disable style output]\n\n", style_h2);
    out.set_base_style(null_style);

    dout << "enabled: ";
    out << fg(blue) << "lorem" << (fg(green) | bold | underline | invert) << "ipsum" << pop << "dolor" << pop << "sit\n";

    dout << "disabled: ";
    out.enable_style(false);
    out << fg(blue) << "lorem" << (bold | underline) << "ipsum" << pop << "dolor" << pop << "sit\n";
}
