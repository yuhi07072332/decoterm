#include <decoterm/decoterm.hpp>

#include <print>

auto main() -> int {
    // combine styles using `|`
    constexpr deco::Style info = deco::bold | deco::fg(deco::bright_cyan);
    constexpr deco::Style warning = deco::bold | deco::fg(deco::yellow);
    constexpr deco::Style error = deco::bold | deco::fg(deco::rgb(255, 85, 85));

    // Output `Style`
    std::println("{}[INFO]{} server started", info, deco::reset);

    // Styled values similar to {fmt} library.
    // You can also use `value | style` instead of `styled(value, style)`.
    std::println("{} cache is nearly full", deco::styled("[WARN]", warning));
    std::println("{} request failed: {}", "[ERROR]" | error, 503 | deco::bold);

    // `init()` will detect terminal environment and initialize the global config.
    deco::TerminalInfo term = deco::init();

    // Modify the global config to disable style output if stdout is redirected.
    deco::config().enable_style(term.is_stdout_tty);

    // `Printer`: similar to `OStream` but uses `std::print` like API.
    // `stdout_printer()` wraps `std::cout` with the global config.
    deco::Printer log = deco::stdout_printer();

    // nested style
    log
        .push(info)
        .print("[INFO] processed {} records in ", 
               128 | deco::bold | deco::fg(deco::bright_green))
        .push(deco::underline | deco::fg(deco::yellow))
        .print("{} ms", 42)
        .pop()
        .print("\n")
        .pop();
}
