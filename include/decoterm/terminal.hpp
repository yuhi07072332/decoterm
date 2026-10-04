// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_TERMINAL_HPP
#define DECOTERM_TERMINAL_HPP

#include "output.hpp"

#include <cstdlib>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#else // POSIX
#include <unistd.h>

#endif

namespace deco {

namespace detail {

inline constexpr auto contains(std::string_view sv, std::string_view find)
    -> bool {
    return sv.find(find) != std::string_view::npos; // NOLINT
}

#if defined(_WIN32)
[[nodiscard]]
inline auto enable_virtual_terminal_mode() -> bool {
    HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(h_stdout, &mode)) return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(h_stdout, mode);
}
#endif // _WIN32

[[nodiscard]]
inline auto no_color() -> bool {
    const char* no_color = std::getenv("NO_COLOR");
    return no_color != nullptr && no_color[0] != '\0';
}

// Based on https://github.com/termstandard/colors?tab=readme-ov-file
// and https://no-color.org/
[[nodiscard]]
inline auto get_color_support() -> ColorMode {
    if (no_color()) return ColorMode::Disabled;

#if defined(_WIN32)
    return ColorSupport::true_color;
#else // POSIX
    const char* colorterm_p = std::getenv("COLORTERM");

    std::string_view env_colorterm = colorterm_p ? colorterm_p : "";
    if (contains(env_colorterm, "truecolor")
        || contains(env_colorterm, "24bit"))
        return ColorMode::TrueColor;

    // TODO:

    return ColorMode::Color16;
#endif
};

/// Checks whether stdout is pointed to a terminal.
[[nodiscard]]
inline auto is_stdout_tty() -> bool {
#if defined(_WIN32)
    HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    return GetConsoleMode(h_stdout, &mode);
#else // POSIX
    return isatty(STDOUT_FILENO);
#endif
};

/// Check whether stderr is pointed to a terminal.
[[nodiscard]]
inline auto is_stderr_tty() -> bool {
#if defined(_WIN32)
    HANDLE h_stderr = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode;
    return GetConsoleMode(h_stderr, &mode);
#else // POSIX
    return isatty(STDERR_FILENO);
#endif
};


} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                        terminal                         ║
// ╚═════════════════════════════════════════════════════════╝

class Terminal {
  public:
    explicit Terminal()
        : color_mode_(detail::get_color_support()),
          is_stdout_tty_(detail::is_stdout_tty()),
          is_stderr_tty_(detail::is_stderr_tty()) {}

    [[nodiscard]] auto color_mode() const -> ColorMode { return color_mode_; }
    [[nodiscard]] auto is_stdout_tty() const -> bool { return is_stdout_tty_; }
    [[nodiscard]] auto is_stderr_tty() const -> bool { return is_stderr_tty_; }

  private:
    ColorMode color_mode_;
    bool is_stdout_tty_;
    bool is_stderr_tty_;
};

/// @brief Initializes terminal.
///
/// @throws (Windows) `std::runtime_error` if it failed to enable virtual terminal mode
inline auto init_terminal() -> Terminal {
#if defined(_WIN32)
    if (!detail::enable_virtual_terminal_mode())
        throw std::runtime_error(
            "deco::init_terminal(): failed to enable virtual terminal mode");
#endif // _WIN32

    auto term = Terminal();
    global_cfg.set_color_mode(term.color_mode());
    return term;
}

} // namespace deco

#endif // !DECOTERM_TERMINAL_HPP
