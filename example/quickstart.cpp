#include "check_print.hpp"

#include <decoterm/decoterm.hpp>

#include <iostream>
#include <string_view>

// clang-format off

auto main() -> int {
    using namespace deco;

    // -- 1. Basic Style output

    std::cout << bold << "Hello ,"
              << (italic | fg(bright_green)) << "world!"
              << reset      // Reset to default style
              << "\n\n";

    Style warning_style = bold | fg(rgb(0xfff2b2));

    // same as fmt::styled() from fmtlib
    std::cout << styled("warning: ", warning_style)
              << "insert warning message here"
              << '[' << styled("-Wunused-variable", warning_style) << ']'
              << '\n';

    // HSV
    std::string_view text = "rainbowwwww~~~~";
    for (std::size_t i = 0, len = text.size(); i < len; ++i) 
        std::cout << fg(hsv(i * (360 / len), 120, 255))
                  << text[i];

    std::cout << '\n';

    // -- 2. Advanced stateful output

    // StyledOstream: TODO: 
    StyledOstream sout = styled_out(std::cout)
        .set_base_style(bg(rgb(32, 32, 32)))
        .enable_style(terminal::is_stdout_tty()) // disable style output when stdout is not a tty
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

#ifdef DECO_HAS_STD_PRINT
    StyledFormat sfmt = styled_fmt();
    sfmt.print(fg(colors::aquamarine1), 
               "{} and {} support\n",
               styled("std::format", italic),
               styled("std::print", italic));
#endif


}
