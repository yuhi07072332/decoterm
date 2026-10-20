#include <decoterm/decoterm.hpp>

#include <print>

auto main() -> int {
    using namespace deco::colors;
    using namespace deco::styles;

    // combine styles using `|`
    constexpr deco::Style info = bold | fg(bright_cyan);
    constexpr deco::Style warning = bold | fg(yellow);
    constexpr deco::Style error = bold | fg(rgb(255, 85, 85));

    // output `Style`
    std::println("{}[INFO]{} server started", info, deco::reset);

    // Styled values similar to {fmt} library.
    // You can also use `value | style` instead of `styled(value, style)`.
    std::println("{} cache is nearly full", deco::styled("[WARN]", warning));
    std::println("{} request failed: {}", "[ERROR]" | error, 503 | bold);

    // `init()` will detect terminal environment and initialize the global config.
    deco::TerminalInfo term = deco::init();

    // Modify the global config to disable style output if stdout is redirected.
    deco::config().enable_style(term.is_stdout_tty);

    // `Printer`: similar to `OStream` but uses `std::print` like API.
    deco::Printer log = deco::stdout_printer();

    // nested style
    log
        .push(info)
        .print("[INFO] processed {} records in ", 
               128 | bold | fg(bright_green))
        .push(underline | fg(yellow))
        .print("{} ms", 42)
        .pop()
        .print("\n")
        .pop();
}
