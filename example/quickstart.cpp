#include <decoterm/decoterm.hpp>

#include <iostream>

// clang-format off

auto main() -> int {
    using namespace deco;

    // -- 1. Basic Style output

    Style warning_style = bold | fg(rgb(0xfff2b2));

    std::cout   << (bold | fg(bright_blue))
                << "Hello "
                << (underline | fg(bright_green))
                << "World!"
                << reset      // Reset to default style
                << "\n\n";

    // same as fmt::styled() from fmtlib
    std::cout << styled("warning: ", warning_style)
              << "insert warning message here"
              << styled("[-Wunused-variable]", warning_style | italic)
              << '\n';

    // -- 2. Advanced stateful output

    StyledOstream sout = styled_ostream(std::cout)
        .set_base_style(bg(rgb(32, 32, 32)))
        .enable_style(terminal::is_stdout_tty()) // disable style output when stdout is not a tty
        .enable_nesting(true);

    // Style nesting
    sout << "base style "
             << fg(bright_blue) << "{ blue "
                 << italic << "{ italic blue "
                     << fg(bright_red) << "{ italic red } "
                 << pop << "italic blue } "
             << pop << "blue } "
         << pop << "base style";
}
