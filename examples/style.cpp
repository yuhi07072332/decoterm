#include <decoterm/style.hpp>
#include <decoterm/format.hpp>

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
    constexpr deco::Style h1 = deco::color(deco::black, deco::bright_green) | deco::bold;

    std::println("{}", deco::styled(h1, "Styles"));
    std::println();

    for (int i = 0; i < style_names.size(); ++i) {
        std::println("{}", deco::styled(deco::Style(1 << i), style_names[i]));
    }
}
