// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECO_TERMINAL_HPP
#define DECO_TERMINAL_HPP

#include "styled_out.hpp"

#include <cstdlib>
#include <iostream>

#if defined(_WIN32)

#include <windows.h>

#else // POSIX

#include <unistd.h>

#endif

namespace deco {

/* ----- forward declarations ----- */

enum class ColorSupport { TrueColor, Color16 };

namespace terminal {

inline auto is_stdout_tty() -> bool;
inline auto is_stderr_tty() -> bool;

} // namespace terminal

namespace detail {

inline constexpr auto contains(std::string_view sv, std::string_view find)
    -> bool {
    return sv.find(find) != std::string_view::npos;                //NOLINT
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

// see https://github.com/termstandard/colors?tab=readme-ov-file,
[[nodiscard]]
inline auto get_color_support() -> ColorSupport {
#if defined(_WIN32)
    return ColorSupport::TrueColor;
#else // POSIX
    const char* colorterm_p = std::getenv("COLORTERM");

    std::string_view env_colorterm = colorterm_p ? colorterm_p : "";
    if (contains(env_colorterm, "truecolor")
        || contains(env_colorterm, "24bit"))
        return ColorSupport::TrueColor;

    return ColorSupport::Color16;
#endif
};

// HACK: same hack used in termcolor library to check if the ostream is stdout
// or stderr
inline auto is_ostream_stdout(const std::ostream& os) -> bool {
    return &os == &std::cout;
}

inline auto is_ostream_stderr(const std::ostream& os) -> bool {
    return (&os == &std::cerr) || (&os == &std::clog);
}

#if defined(_WIN32)
inline bool g_virtual_terminal_mode_enabled = enable_virtual_terminal_mode();
#endif // _WIN32

} // namespace detail

namespace terminal {

/// @brief Check if the current stdout is a terminal.
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

/// @brief Check if the current stderr is a terminal.
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

/// @brief Get the color support of the current terminal.
[[nodiscard]]
inline auto color_support() -> ColorSupport {
    static ColorSupport s_color_support = detail::get_color_support();
    return s_color_support;
}

/// @brief Same as `deco::styled_out()` but with automatic configuration
[[nodiscard]]
inline auto styled_out(std::ostream& os) -> StyledOstream {
    bool is_tty = (detail::is_ostream_stdout(os) && is_stdout_tty())
                  || (detail::is_ostream_stderr(os) && is_stderr_tty());
    return deco::styled_ostream(os).enable_style(is_tty).enable_context(is_tty);
}

}; // namespace terminal

} // namespace deco

#endif // !DECO_TERMINAL_HPP
