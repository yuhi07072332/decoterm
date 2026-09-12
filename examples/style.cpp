#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <array>
#include <string_view>

constexpr std::array<std::string_view, 8> style_names {
    "bold",
    "dim",
    "italic",
    "underline",
    "blink",
    "invert",
    "strikethrough",
    "underline_double",
};

auto main() -> int {
    std::println("{}",
                 (deco::color(deco::black, deco::bright_green) | deco::bold)
                     % "Styles");
    std::println();

    for (int i = 0; i < style_names.size(); ++i) {
        std::println("{}", deco::Style(1 << i) % style_names[i]);
    }
}
