#include <decoterm/decoterm.hpp>
#include <iostream>

int main() {
    using namespace deco;

    constexpr Style style_info = fg(rgb(0x71cfde)) | underline;

    std::cout << fg(bright_blue) << "Hello, " << invert << "world!" << reset;

    StyledOstream sout(std::cout);

    sout        << style_info
                << (invert | bold) << "INFO" << pop   // nested style
                << " pop is same as reset if style stack is not enabled.\n"
                << pop;

    std::cout   << '\n';

}
