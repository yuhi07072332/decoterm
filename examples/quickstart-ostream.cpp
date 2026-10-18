#include <decoterm/decoterm.hpp>

#include <iostream>

auto main() -> int {
    // combine styles using `|`
    constexpr deco::Style info = deco::bold | deco::fg(deco::bright_cyan);
    constexpr deco::Style warning = deco::bold | deco::fg(deco::yellow);
    constexpr deco::Style error = deco::bold | deco::fg(deco::rgb(255, 85, 85));

    // Output `Style`
    std::cout << info << "[INFO]" << deco::reset << " server started\n";

    // Styled values similar to {fmt} library.
    // You can also use `value | style` instead of `styled(value, style)`.
    std::cout << deco::styled("[WARN]", warning) << " cache is nearly full\n";
    std::cout << ("[ERROR]" | error) << " request failed: " << (503 | deco::bold) << '\n';

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
        << (128 | deco::bold | deco::fg(deco::bright_green))
        << " records in "
        << deco::push(deco::underline | deco::fg(deco::yellow))
        << 42 << " ms"
        << deco::pop
        << '\n'
        << deco::pop;
}
