#include <decoterm/decoterm.hpp>
#include <iostream>

int main() {
    using namespace deco;

    static_assert(detail::OutputableStyle<Style>);

    style_stack(true);

    constexpr Style style_info = fg(rgb(0x71cfde)) | underline;

    std::cout << fg(bluelight) << "Hello, " << invert << "world!" << reset;

    std::cout   << style_info
                << (invert | bold) << "INFO" << pop   // nested style
                << " pop is same as reset if style stack is not enabled.\n"
                << pop;

    std::cout   << '\n';

}
