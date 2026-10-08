// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_TERMINAL_HPP
#define DECOTERM_TERMINAL_HPP

#include "output.hpp"

#include <cstdlib>
#include <string_view>

#if defined(_WIN32)
#include <stdexcept>
#include <windows.h>
#else // POSIX
#include <unistd.h>

#endif

namespace deco {

namespace detail {

#if defined(_WIN32)
[[nodiscard]]
inline auto enable_stdout_vt() -> bool {
    HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_stdout == INVALID_HANDLE_VALUE) return false;
    DWORD mode;
    if (!GetConsoleMode(h_stdout, &mode)) return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(h_stdout, mode);
}

[[nodiscard]]
inline auto enable_stderr_vt() -> bool {
    HANDLE h_stderr = GetStdHandle(STD_ERROR_HANDLE);
    if (h_stderr == INVALID_HANDLE_VALUE) return false;
    DWORD mode;
    if (!GetConsoleMode(h_stderr, &mode)) return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(h_stderr, mode);
}
#endif // _WIN32

[[nodiscard]]
inline auto no_color() -> bool {
    const char* no_color = std::getenv("NO_COLOR");
    return no_color != nullptr && no_color[0] != '\0';
}

// Based on https://github.com/termstandard/colors?tab=readme-ov-file
[[nodiscard]]
inline auto get_color_support() -> ColorSupport {
    auto contains = [](const char* lhs, std::string_view rhs) -> bool {
        std::string_view sv = lhs ? lhs : "";
        return sv.find(rhs) != std::string_view::npos; // NOLINT
    };

    const char* colorterm = std::getenv("COLORTERM");

    if (contains(colorterm, "truecolor")
        || contains(colorterm, "24bit"))
        return ColorSupport::TrueColor;

    const char* term = std::getenv("TERM");
    if (contains(colorterm, "256") || contains(term, "256")) 
        return ColorSupport::Color256;

#if defined(_WIN32)
    // TODO: 
    return ColorSupport::TrueColor;
#else // POSIX

    return ColorSupport::Color16;
#endif
};

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

struct TerminalInfo {
    ColorSupport color_support;
    bool is_stdout_tty;
    bool is_stderr_tty;
    bool no_color;
};

[[nodiscard]]
inline auto terminal_info() -> TerminalInfo {
    return {
        .color_support = detail::get_color_support(),
        .is_stdout_tty = detail::is_stdout_tty(),
        .is_stderr_tty = detail::is_stderr_tty(),
        .no_color = detail::no_color(),
    };
}

/// @brief Configures the global output based on the terminal environment.
///
/// @throws (Windows) `std::runtime_error` if it failed to enable virtual terminal mode
inline auto init() -> TerminalInfo {
    auto info = terminal_info();
    auto& cfg = config();

    cfg.enable_color(!info.no_color);
    cfg.set_color_support(info.color_support);

#if defined(_WIN32)
    if (!detail::enable_stdout_vt() && !info.is_stdout_tty)
        throw std::runtime_error(
            "deco::init(): failed to enable virtual terminal mode");

    if (!detail::enable_stderr_vt() && !info.is_stderr_tty)
        throw std::runtime_error(
            "deco::init(): failed to enable virtual terminal mode");
#endif // _WIN32

    return info;
}

} // namespace deco

#endif // !DECOTERM_TERMINAL_HPP
