#include <decoterm/formatter.hpp>

#include <array>
#include <print>
#include <string_view>

using namespace deco;

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

int main() {
    std::println("Normal");
    for (int i = 0; i < style_names.size(); ++i) {
        std::print(
            "{}{}{}  ",
            Style(static_cast<Style::StyleFlags>(1 << i)),
            style_names[i],
            reset
        );

        std::println();
    }
}
