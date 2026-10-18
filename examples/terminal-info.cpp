#include <decoterm/terminal.hpp>
#include <decoterm/format.hpp>

#include <print>
#include <string>

auto color_support_string(deco::ColorSupport cs) -> std::string {
    switch (cs) {
        case deco::ColorSupport::Color16: return "Color16";
        case deco::ColorSupport::Color256: return "Color256";
        case deco::ColorSupport::TrueColor: return "TrueColor";
    }
    assert(false);
    return "";  // unreachable
}

auto main() -> int {
    const deco::Style highlight = deco::bold | deco::fg(deco::bright_yellow);
    const deco::TerminalInfo term = deco::init();

    std::println("color_support: {}", color_support_string(term.color_support) | highlight);
    std::println("is_stdout_tty: {}", term.is_stdout_tty | highlight);
    std::println("is_stderr_tty: {}", term.is_stderr_tty | highlight);
    std::println("no_color     : {}", term.no_color | highlight);
}
