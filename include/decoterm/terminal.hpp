// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_TERMINAL_HPP
#define DECOTERM_TERMINAL_HPP

#include "decoterm.hpp"

#include <optional>

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX
#include <windows.h>

#else // POSIX

#include <unistd.h>
#include <termios.h>

#endif

namespace deco {

enum class ColorSupport {
    TrueColor,
    Color256,
    Color16
};

namespace detail {

#if defined(_WIN32)
inline auto enable_virtual_terminal_mode() -> bool {
    HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(h_stdout, &mode)) return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(h_stdout, mode);
}
#endif  // _WIN32

[[nodiscard]]
inline auto is_stdout_terminal() -> bool {
#if defined(_WIN32)
    HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    return GetConsoleMode(h_stdout, &mode);
#else // POSIX
    return isatty(STDOUT_FILENO);
#endif
};

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
    //termios original_termios;
    //tcgetattr(STDIN_FILENO, &original_termios);
    //
    //termios raw = original_termios;
    //cfmakeraw(&raw);
    //raw.c_cc[VTIME] = 1;
    //raw.c_cc[VMIN] = 0;
    //tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    //
    //write(STDOUT_FILENO, "\x1b[48;2;1;2;3m\x1bP$qm\x1b\\",20);
    //
    //std::array<char, 24> buf;
    //for (std::size_t i = 0; i < buf.size() - 1; ++i) {
    //    if (read(STDIN_FILENO, &buf[i], 1) != 1
    //        || (i >= 2 && buf[i - 1] == '\\' && buf[i - 2] == '\x1b')) {
    //            buf[i + 1] = '\0';
    //        break;
    //    }
    //}
    //
    //tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    //
    //std::string_view receive(buf.begin());
    //if (receive.starts_with("\x1bP1$r")
    //    && receive.find("48:2:1:2:3m") != receive.npos)
    //    return ColorSupport::TrueColor;
    //
    //return ColorSupport::Color16;
};

#endif  // !_WIN32

}   // namespace detail

enum class StyleMode { CheckStdout, Manual };

struct TerminalOption {
    StyleMode style_mode = StyleMode::CheckStdout;
    bool use_color_fallback = true;
    bool restore_default_style_on_exit = true;
};

class Terminal {
public:
    explicit Terminal(TerminalOption option = TerminalOption()) : option_(option) {
		// enable virtual terminal processing on Windows to support ANSI escape codes
#if defined(_WIN32)
		if (detail::enable_virtual_terminal_mode()) 
		    cashed_color_support_ = ColorSupport::TrueColor;

#else 
        if (option_.use_color_fallback) {
            cashed_color_support_ = detail::get_color_support();
            if (cashed_color_support_ != ColorSupport::TrueColor)
                detail::output_state().set_color_fallback(true);
        }
#endif

        if (option_.style_mode == StyleMode::CheckStdout)
            detail::output_state().set_style(detail::is_stdout_terminal());

    }

    ~Terminal() {
        if (option_.restore_default_style_on_exit) std::cout << reset;
    }

    auto color_support() const -> std::optional<ColorSupport> {
        return cashed_color_support_;
    }

private:
    const TerminalOption option_;
    std::optional<ColorSupport> cashed_color_support_;
};

}   // namespace deco

#endif // !DECOTERM_TERMINAL_HPP
