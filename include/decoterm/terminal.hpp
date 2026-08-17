// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_TERMINAL_HPP
#define DECOTERM_TERMINAL_HPP

#include "output.hpp"

#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#else // POSIX
#include <unistd.h>

#endif

namespace deco {

namespace terminal {

inline auto is_stdout_tty() -> bool;
inline auto is_stderr_tty() -> bool;

} // namespace terminal

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

// Based on https://github.com/termstandard/colors?tab=readme-ov-file
[[nodiscard]]
inline auto get_color_support() -> ColorMode {
#if defined(_WIN32)
    return ColorSupport::true_color;
#else // POSIX
    const char* colorterm_p = std::getenv("COLORTERM");

    std::string_view env_colorterm = colorterm_p ? colorterm_p : "";
    if (contains(env_colorterm, "truecolor")
        || contains(env_colorterm, "24bit"))
        return ColorMode::true_color;

    // TODO:

    return ColorMode::color16;
#endif
};

// HACK: Uses the same approach as the termcolor library to detect whether
// an ostream is stdout or stderr.
inline auto is_ostream_stdout(const std::ostream& os) -> bool {
    return &os == &std::cout;
}

inline auto is_ostream_stderr(const std::ostream& os) -> bool {
    return (&os == &std::cerr) || (&os == &std::clog);
}

} // namespace detail

namespace terminal {

#if defined(_WIN32)
// Automatically enables Windows virtual terminal processing unless
// DECOTERM_NO_AUTO_ENABLE_VT is defined.
#ifndef DECOTERM_NO_AUTO_ENABLE_VT
inline bool g_virtual_terminal_mode_enabled = enable_virtual_terminal_mode();
#endif // DECOTERM_NO_AUTO_ENABLE_VT
#endif // _WIN32

/// @brief Checks whether stdout is pointed to a terminal.
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

/// @brief Check whether stderr is pointed to a terminal.
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

/// @brief Returns the detected color support of teh current terminal.
[[nodiscard]]
inline auto color_support() -> ColorMode {
    static ColorMode s_color_support = detail::get_color_support();
    return s_color_support;
}

/// @brief Creates a StyledOstream with TTY-based automatic configuration.
/// @details Equivalent to `deco::styled_out(os)`, but style output and context
/// tracking are enabled only when os is detected as stdout or stderr attached
/// to a terminal.
[[nodiscard]]
inline auto styled_out(std::ostream& os) -> StyledOstream {
    bool is_tty = (detail::is_ostream_stdout(os) && is_stdout_tty())
                  || (detail::is_ostream_stderr(os) && is_stderr_tty());
    return deco::styled_out(os).enable_style(is_tty).enable_context_tracking(
        is_tty);
}

}; // namespace terminal

} // namespace deco

#endif // !DECOTERM_TERMINAL_HPP
