// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_TERMINAL_HPP
#define DECOTERM_TERMINAL_HPP

#include "styled.hpp"

#include <cstdlib>
#include <iostream>

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX
#include <windows.h>

#else // POSIX

#include <termios.h>
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

#if !defined(_WIN32)

// see https://github.com/termstandard/colors?tab=readme-ov-file,
// https://www.xfree86.org/current/ctlseqs.html
[[nodiscard]]
inline auto get_color_support() -> ColorSupport {
    const char* colorterm_p = std::getenv("COLORTERM");

    std::string_view env_colorterm = colorterm_p ? colorterm_p : "";
    if (env_colorterm == "truecolor" || env_colorterm == "24bit")
        return ColorSupport::TrueColor;

    return ColorSupport::Color16;

    ////fallback to DECRQSS if $COLORTERM is not set.
    //
    // termios original_termios;
    // tcgetattr(STDIN_FILENO, &original_termios);
    //
    // termios raw = original_termios;
    // cfmakeraw(&raw);
    // raw.c_cc[VTIME] = 1;
    // raw.c_cc[VMIN] = 0;
    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    //
    // write(STDOUT_FILENO, "\x1b[48;2;1;2;3m\x1bP$qm\x1b\\",20);
    //
    // std::array<char, 24> buf;
    // for (std::size_t i = 0; i < buf.size() - 1; ++i) {
    //    if (read(STDIN_FILENO, &buf[i], 1) != 1
    //        || (i >= 2 && buf[i - 1] == '\\' && buf[i - 2] == '\x1b')) {
    //            buf[i + 1] = '\0';
    //        break;
    //    }
    //}
    //
    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    //
    // std::string_view receive(buf.begin());
    // if (receive.starts_with("\x1bP1$r")
    //    && receive.find("48:2:1:2:3m") != receive.npos)
    //    return ColorSupport::TrueColor;
    //
    // return ColorSupport::Color16;
};

#endif // !_WIN32

inline auto is_ostream_tty(const std::ostream& os) -> bool {
    if (&os == &std::cout) {
        return terminal::is_stdout_tty();
    } else if (&os == &std::cerr) {
        return terminal::is_stderr_tty();
    }
    return false;
}

#if defined(_WIN32)
inline bool g_virtual_terminal_mode_enabled = enable_virtual_terminal_mode();
#endif // _WIN32

} // namespace detail

namespace terminal {

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

[[nodiscard]]
inline auto cashed_color_support() -> ColorSupport {
    static ColorSupport color_support = detail::get_color_support();
    return color_support;
}

[[nodiscard]]
inline auto styled_out(std::ostream& os) -> StyledOstream {
    StyledOstream sout = StyledOstream(os);
    sout.enable_style(detail::is_ostream_tty(os))
        .enable_color_fallback(cashed_color_support() == ColorSupport::Color16);
    return sout;
}

}; // namespace terminal

} // namespace deco

#endif // !DECOTERM_TERMINAL_HPP
