#include <decoterm/format.hpp>
#include <decoterm/deco.hpp>

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
                 "Styles" |
                     (deco::color(deco::black, deco::bright_green) | deco::bold));
    std::println();

    for (std::size_t i = 0; i < style_names.size(); ++i) {
        std::println("{}", style_names[i] | deco::Style(1 << i));
    }
}
