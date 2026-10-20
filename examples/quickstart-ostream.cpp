#define DECOTERM_NO_FORMAT
#include <decoterm/decoterm.hpp>

#include <iostream>

auto main() -> int {
    using namespace deco::colors;
    using namespace deco::styles;

    // combine styles using `|`
    constexpr deco::Style info = bold | fg(bright_cyan);
    constexpr deco::Style warning = bold | fg(yellow);
    constexpr deco::Style error = bold | fg(rgb(255, 85, 85));

    // output `Style`
    std::cout << info << "[INFO]" << deco::reset << " server started\n";

    // Styled values similar to {fmt} library.
    // You can also use `value | style` instead of `styled(value, style)`.
    std::cout << deco::styled("[WARN]", warning) << " cache is nearly full\n";
    std::cout << ("[ERROR]" | error) << " request failed: " << (503 | bold) << '\n';

    // `init()` will detect terminal environment and initialize the global config.
    deco::TerminalInfo term = deco::init();

    // Modify the global config to disable style output if stdout is redirected.
    deco::config().enable_style(term.is_stdout_tty);

    // `OStream`: a `std::ostream` wrapper that tracks style context and uses
    //  the global config.
    // `stdout_ostream()` wraps `std::cout` with the global config.
    deco::OStream log = deco::stdout_ostream();

    // nested style
    log << deco::push(info)
        << "[INFO] processed "
        << (128 | bold | fg(bright_green))
        << " records in "
        << deco::push(underline | fg(yellow))
        << 42 << " ms"
        << deco::pop
        << '\n'
        << deco::pop;
}
