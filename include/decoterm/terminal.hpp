#ifndef DECOTERM_TEMINAL_HPP
#define DECOTERM_TEMINAL_HPP

#ifndef DECOTERM_NO_GLOBAL_TERMINAL
#define DECOTERM_DETAIL_GLOBAL_TERMINAL
#endif // !DECOTERM_NO_GLOBAL_TERMINAL

#include "decoterm.hpp"

#include <iostream>
#include <vector>

#if defined(__linux__) || defined(__unix__)

#include <unistd.h>
#include <termios.h>

#endif // defined(__linux__) || defined(__unix__)

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                        Terminal                         ║
// ╚═════════════════════════════════════════════════════════╝

enum class ColorSupport {
    TrueColor,
    Color16
};

namespace detail {

struct stylepop_t {};


/// @brief make writter write to buffer, and emit to ostream.
/// @detail max buffer size is Style::MAX_ESCAPE_CODE_SIZE + 1
/// @param writter function: (OutputIt) -> OutputIt, where OutputIt is char*
inline void write_style_to_ostream(std::invocable<char*> auto&& writter,
                         std::ostream& os) {
    std::array<char, Style::MAX_ESCAPE_CODE_SIZE + 1> buf;
    auto out = writter(buf.begin());
    os.write(buf.data(), out - buf.begin());
}

inline void write_stdout(std::string_view s) {
    write(STDOUT_FILENO, s.data(), s.size());
}

[[nodiscard]]
inline auto is_stdout_tty() -> bool {
    return isatty(STDOUT_FILENO);
};

// see https://github.com/termstandard/colors?tab=readme-ov-file,
// https://www.xfree86.org/current/ctlseqs.html
[[nodiscard]]
inline auto get_color_support() -> ColorSupport {
    auto sv_from_pointer = [](const char* ch) -> std::string_view {
        if (ch) return ch;
        else return "";
    };

    std::string_view colorterm = sv_from_pointer(std::getenv("COLORTERM"));
    if (colorterm == "truecolor" || colorterm == "24bit")
        return ColorSupport::TrueColor; 

    // fallback to DECRQSS if $COLORTERM is not set.

    termios original_termios;
    tcgetattr(STDIN_FILENO, &original_termios);

    termios raw = original_termios;
    cfmakeraw(&raw);
    raw.c_cc[VTIME] = 1;
    raw.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    write(STDOUT_FILENO, "\x1b[48;2;1;2;3m\x1bP$qm\x1b\\",20);

    std::array<char, 24> buf;
    for (std::size_t i = 0; i < buf.size() - 1; ++i) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1
            || (i >= 2 && buf[i - 1] == '\\' && buf[i - 2] == '\x1b')) {
                buf[i + 1] = '\0';
            break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);

    std::string_view receive(buf.begin());
    if (receive.starts_with("\x1bP1$r")
        && receive.find("48:2:1:2:3m") != receive.npos)
        return ColorSupport::TrueColor;

    return ColorSupport::Color16;
};

}   // namespace detail


class Terminal {
public:
    // ----- options -----
    enum class StyleMode{ CheckStdout, Always, Never };
    
    /// @brief set style output mode. [default: CheckStdout]
    /// @details 
    /// StyleMode::CheckStdout: if current stdout points to tty, enable.
    /// StyleMode::Always:      always enable style output.
    /// StyleMode::Never:       disable style output.
    auto style_mode(StyleMode set) -> Terminal& {
        is_style_enabled_ = set == StyleMode::Always
            || (set == StyleMode::CheckStdout && detail::is_stdout_tty());
        return *this;
    }

    /// @brief [default: true]
    auto restore_default_style_on_exit(bool set) -> Terminal& {
        restore_default_style_on_exit_ = set;
        return *this;
    }

    /// @brief [default: true]
    auto enable_style_track(bool set) -> Terminal& {
        is_style_track_enabled_ = set;
        return *this;
    }

    auto enable_color_fallback(bool set) -> Terminal& {
        is_color_fallback_enabled_ = set;
        if (set) color_support_ = detail::get_color_support();
        return *this;
    }

    // ----- constructor -----

    Terminal() {
        style_mode(StyleMode::CheckStdout);
        if (is_color_fallback_enabled_) 
            color_support_ = detail::get_color_support();
    }

    ~Terminal() {
        //TODO: 
        if (restore_default_style_on_exit_) std::cout << reset.to_escape();
    }

    auto current_style() const -> Style { 
        return !style_track_.empty() ? style_track_.back().style
            : default_style_.style;
    }

    auto is_style_enabled() const -> bool { return is_style_enabled_; }

    auto color_support() const -> ColorSupport { return color_support_; }

private:
    friend class std::formatter<Style>;
    friend class std::formatter<AbsoluteStyle>;
    friend class std::formatter<detail::stylepop_t>;

    template <detail::OutputableStyle StyleType>
    friend auto operator<<(std::ostream& os, StyleType rhs) -> std::ostream&;
    friend auto operator<<(std::ostream& os, detail::stylepop_t) -> std::ostream&;

    template <std::output_iterator<const char&> OutputIt>
    auto style_push_and_apply(OutputIt out, AbsoluteStyle abs) -> OutputIt {
        //TODO: check if style is same as current_style()
        if (is_style_track_enabled_) style_track_.push_back(abs.style);
        if (is_style_enabled_) return abs.to_escape(out);
        else return out;
    }

    template <std::output_iterator<const char&> OutputIt>
    auto style_push_and_apply(OutputIt out, Style style) -> OutputIt {
        if (is_style_track_enabled_) {
            AbsoluteStyle absolute(current_style());
            absolute.style |= style;
            style_track_.push_back(absolute);
        }
        if (is_style_enabled_) return style.to_escape(out);
        else return out;
    }

    template <std::output_iterator<const char&> OutputIt>
    auto style_pop_and_apply(OutputIt out) -> OutputIt {
        if (is_style_track_enabled_) style_track_.pop_back();
        if (is_style_enabled_) return current_style().to_escape(out);
        else return out;
    }

    std::vector<AbsoluteStyle> style_track_;
    AbsoluteStyle default_style_ = AbsoluteStyle();

    bool restore_default_style_on_exit_ = true;
    bool is_style_track_enabled_ = true;
    bool is_color_fallback_enabled_ = true;

    ColorSupport color_support_;
    bool is_style_enabled_;
};

inline constexpr detail::stylepop_t pop;

#ifdef DECOTERM_DETAIL_GLOBAL_TERMINAL

/// the global terminal object.
inline Terminal terminal = Terminal();

template <detail::OutputableStyle StyleType>
inline auto operator<<(std::ostream& os, StyleType rhs) -> std::ostream& {
    detail::write_style_to_ostream([rhs](char* out){
        return terminal.style_push_and_apply(out, rhs);
    }, os);
    return os;
}

inline auto operator<<(std::ostream& os, detail::stylepop_t) -> std::ostream& {
    detail::write_style_to_ostream([](char* out) {
        return terminal.style_pop_and_apply(out);
    }, os);
    return os;
}

#endif // DECOTERM_DETAIL_GLOBAL_TERMINAL

}   // namespace deco

#endif // !DECOTERM_TEMINAL_HPP
