#include <decoterm/decoterm.hpp>

#include <iostream>
#include <string_view>

// clang-format off

auto main() -> int {
    using namespace deco;

    // -- 1. Basic Style Output

    std::cout << bold << "Hello ,"
              << (italic | fg(bright_green)) << "world!"
              << reset      // reset to default style
              << "\n\n";

    // Style composition
    const Style warning_style = bold | fg(rgb(0xfff2b2));

    // Similar to fmt::styled() from fmtlib
    std::cout << styled("warning: ", warning_style)
              << "Unused variable 'x'"
              << styled("[-Wunused-variable]", warning_style)
              << '\n';

    // HSV usage
    std::string_view text = "rainbowwwww~~~~";
    for (std::size_t i = 0, len = text.size(); i < len; ++i) 
        std::cout << fg(hsv(i * (360 / len), 120, 255))
                  << text[i];

    std::cout << '\n';

    // -- 2. Advanced stateful output

    // StyledOstream: ostream wrapper with style output states.
    StyledOstream sout = styled_out(std::cout)
        .set_base_style(bg(rgb(32, 32, 32)))
        //Enable style output only when stdout is a TTY.
        .enable_style(terminal::is_stdout_tty())
        .enable_nesting();

    // Style nesting
    sout << "base style "
             << fg(bright_blue) << "{ blue "
                 << italic << "{ italic blue "
                     << fg(bright_red) << "{ italic red } "
                 << pop << "italic blue } "
             << pop << "blue } "
         << pop << "base style";

    std::cout << '\n';

    // StyledFormat: Basically the same as StyledOstream, but uses format‑based output APIs.
    StyledFormat sfmt = fstyled_out();
    sfmt.print(fg(colors::aquamarine1), 
               "{} and {} support\n",
               styled("std::format", italic),
               styled("std::print", italic));
}
