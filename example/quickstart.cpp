#include <decoterm/decoterm.hpp>
#include <iostream>

int main() {
    deco::enable_style_stack();

    constexpr deco::Style style_info = deco::fg(deco::rgb(0x71cfde)) | deco::underline;
    constexpr deco::Style style_warning = deco::fg(deco::yellowlight)
        | deco::bg(deco::rgb(47, 45, 37))
        | deco::bold | deco::italic;

    std::cout   << style_info
                << (deco::invert | deco::bold) << "INFO" << deco::pop   // nested style
                << " deco::pop is same as deco::reset if style stack is not enabled.\n"
                << deco::pop;

    std::cout   << '\n';

    std::cout   << style_warning
                << "WARNING: constexpr hsv() is not supported before C++23.\n"
                << deco::pop;
}
