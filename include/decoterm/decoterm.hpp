// Terminal styling library for C++20
//
// SPDX-License-Identifier: MIT
// MIT Licence
//
// Copyright (c) 2026 Yuhi0707
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef DECOTERM_DECOTERM_HPP
#define DECOTERM_DECOTERM_HPP

// TODO:
// - Detect terminal color support info & provide fallback system color for true color
// - styled()
// - markup string
// - Windows API fallback for Windows8 or lower versions

#include <array>
#include <concepts>
#include <iostream>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace deco {

namespace detail {

template <typename T>
concept OutputableStyle = requires (T style, char* out) {
    { T::MAX_ESCAPE_CODE_SIZE } -> std::convertible_to<const std::size_t>;
    { style.to_escape(out) } -> std::convertible_to<char*>;
};

struct default_color_t {};
struct null_color_t {};

// clang-format off
inline constexpr std::array<std::string_view, 16> SGR_PARAM_FG {
    "30", "31", "32",
    "33", "34", "35",
    "36", "37", "90",
    "91", "92", "93",
    "94", "95", "96",
    "97"
};

inline constexpr std::array<std::string_view, 16> SGR_PARAM_BG {
    "40", "41", "42",
    "43", "44", "45",
    "46", "47", "100",
    "101", "102", "103",
    "104", "105", "106",
    "107"
};

inline constexpr std::array<std::string_view, 8> SGR_PARAM_STYLE {
    "1" /*bold*/,           "2"  /*dim*/,           "3" /*italic*/,
    "4" /*underline*/,      "5"  /*blink*/,         "7" /*invert*/,
    "9" /*strikethrough*/,  "21" /*double underline*/
};
// clang-format on

template <std::output_iterator<const char&> OutputIt>
inline constexpr auto write_to(OutputIt out, std::string_view sv) -> OutputIt {
    for (auto c : sv) *out++ = c;
    return out;
}

}   // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                          Color                          ║
// ╚═════════════════════════════════════════════════════════╝


struct Color {
    enum class Type : uint8_t { Null = 0, Default, Colors, TrueColor };

    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE = 19;

    // ----- constructors -----

    /// @brief Create Color::None
    constexpr Color();

    constexpr Color(detail::default_color_t) : type_(Type::Default), data_({0, 0, 0}) {}
    constexpr Color(detail::null_color_t) : type_(Type::Null), data_({0, 0, 0}) {}

    constexpr explicit Color(uint8_t index)
        : type_(Type::Colors), data_({index, 0, 0}) {}

    constexpr explicit Color(uint8_t r, uint8_t g, uint8_t b)
        : type_(Type::TrueColor), data_({r, g, b}) {}

    // ----- operators -----

    constexpr explicit operator bool() const { return !is_empty(); }

    constexpr auto operator==(const Color&) const -> bool = default;
    constexpr auto operator!=(const Color&) const -> bool = default;

    // ----- observe -----

    constexpr auto is_empty() const -> bool { return type_ == Type::Null; }
    constexpr auto type() const -> Type { return type_; }

    // ----- output -----
    
    /// @brief write SGR parameters to out.
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_sgr_params(OutputIt out, bool is_bg) const -> OutputIt {
        using namespace detail;
        auto write_converted = [&out](int value) {
            std::array<char, 3> buf;
            auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + 3, value);
            assert(ec == std::errc{});
            auto len = ptr - buf.data();
            for (int i = 0; i < len; ++i)
                *out++ = buf[i];
        };

        switch (type_) {
            case Type::Null :
                return out;
            case Type::Default :
                if (is_bg) out = write_to(out, "49");
                else out = write_to(out, "39");
                return out;
            case Type::Colors : {
                if (data_[0] < 16) {
                    out = write_to(out, is_bg ? SGR_PARAM_BG[data_[0]]
                                 : SGR_PARAM_FG[data_[0]]);
                } else {
                    out = write_to(out, is_bg ? "48;5;" : "38;5;");
                    write_converted(data_[0]);
                }
                return out;
            }
            case Type::TrueColor : {
                const auto [r, g, b] = data_;
                out = write_to(out, is_bg ? "48;2;" : "38;2;");
                write_converted(r); *out++ = ';';
                write_converted(g); *out++ = ';';
                write_converted(b);
                return out;
            }

            default: return out;
        }
    }

    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out, bool is_bg) const -> OutputIt {
        out = detail::write_to(out, "\x1b[");
        out = to_sgr_params(out, is_bg);
        *out++ = 'm';
        return out;
    }

    /// @brief generate full escape code
    [[nodiscard]]
    auto to_escape(bool is_bg) const -> std::string {
        std::string esc;
        to_escape(std::back_inserter(esc), is_bg);
        return esc;
    }

private:
    Type type_;
    std::array<uint8_t, 3> data_;
};

/// @brief Create a color by RGB
inline constexpr auto rgb(uint8_t r, uint8_t g, uint8_t b) -> Color {
    return Color(r, g, b);
}

/// @brief Create a color by 0xRRGGBB
/// @pre rgb <= 0xFFFFFF
inline constexpr auto rgb(uint32_t hex) -> Color {
    // clang-format off
    if (hex > 0xffffff) 
        throw std::invalid_argument("deco::Color::rgb(): rgb > 0xffffff");
    return Color((hex >> 16) & 0xFF,
                    (hex >> 8) & 0xFF,
                    hex & 0xFF);
    // clang-format on
}

/// @brief Create a color by HSV
/// @param h [0, 360): Hue of the color
/// @param s [0, 255]: Saturation of the color
/// @param v [0, 255]: Value (brightness) of the color
[[nodiscard]]
inline constexpr auto hsv(uint16_t h, uint8_t s, uint8_t v) -> Color {
    // clang-format off
    if (h < 0 || h >= 360) throw std::invalid_argument(
        "deco::Color::hsv(): h is not in range [0, 360)");

    float hp = h / 60.f;
    float sp = s / 255.0f;
    float vp = v / 255.0f;

    float f = hp - std::floor(hp);
    uint8_t p = std::round(vp * (1 - sp) * 255);
    uint8_t q = std::round(vp * (1 - f * sp) * 255);
    uint8_t t = std::round(vp * (1 - (1 - f) * sp) * 255);

    switch (h / 60) {
        case 0 : return Color(v, t, p);
        case 1 : return Color(q, v, p);
        case 2 : return Color(p, v, t);
        case 3 : return Color(p, q, v);
        case 4 : return Color(t, p, v);
        case 5 : return Color(v, p, q);
        default: assert(false);
    }
    // clang-format on
}

namespace colors {

/// system colors
enum Color16 : uint8_t {
    Black             = 0,
    Red               = 1,
    Green             = 2,
    Yellow            = 3,
    Blue              = 4,
    Magenta           = 5,
    Cyan              = 6,
    White             = 7,
    BlackLight        = 8,
    RedLight          = 9,
    GreenLight        = 10,
    YellowLight       = 11,
    BlueLight         = 12,
    MagentaLight      = 13,
    CyanLight         = 14,
    WhiteLight        = 15,
};

}   // namespace colors

// ----- color constants -----

inline constexpr Color default_color   = Color(detail::default_color_t{});
inline constexpr Color null_color      = Color(detail::null_color_t{});

inline constexpr Color black        = Color(colors::Black);
inline constexpr Color red          = Color(colors::Red);
inline constexpr Color green        = Color(colors::Green);
inline constexpr Color yellow       = Color(colors::Yellow);
inline constexpr Color blue         = Color(colors::Blue);
inline constexpr Color magenta      = Color(colors::Magenta);
inline constexpr Color cyan         = Color(colors::Cyan);
inline constexpr Color white        = Color(colors::White);
inline constexpr Color blacklight   = Color(colors::BlackLight);
inline constexpr Color redlight     = Color(colors::RedLight);
inline constexpr Color greenlight   = Color(colors::GreenLight);
inline constexpr Color yellowlight  = Color(colors::YellowLight);
inline constexpr Color bluelight    = Color(colors::BlueLight);
inline constexpr Color magentalight = Color(colors::MagentaLight);
inline constexpr Color cyanlight    = Color(colors::CyanLight);
inline constexpr Color whitelight   = Color(colors::WhiteLight);

// ╔═════════════════════════════════════════════════════════╗
// ║                          Style                          ║
// ╚═════════════════════════════════════════════════════════╝

struct Style {
    enum Flags : uint8_t {
        None            = 0,
        Bold            = 1 << 0,
        Dim             = 1 << 1,
        Italic          = 1 << 2,
        Underline       = 1 << 3,
        Blink           = 1 << 4,
        Invert          = 1 << 5,
        Strikethrough   = 1 << 6,
        UnderlineDouble = 1 << 7,
    };

    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE = 
        Color::MAX_ESCAPE_CODE_SIZE * 2 + 13 + 3;

    uint8_t flags   = None;
    Color fg        = null_color;
    Color bg        = null_color;

    constexpr Style() = default;

    constexpr Style(uint8_t flags, Color fg = null_color, Color bg = null_color)
        : flags(flags), fg(fg), bg(bg) {}

    // ----- operators -----

    constexpr void operator|=(Style rhs) {
        flags = flags | rhs.flags;
        fg = rhs.fg ? rhs.fg : fg;
        bg = rhs.bg ? rhs.bg : bg;
    }

    constexpr auto operator|(Style rhs) const -> Style {
        return Style(
            flags | rhs.flags,
            rhs.fg ? rhs.fg : fg,
            rhs.bg ? rhs.bg : bg
        );
    }

    constexpr auto operator==(const Style&) const -> bool = default;
    constexpr auto operator!=(const Style&) const -> bool = default;

    constexpr explicit operator bool() const { return !is_empty(); }

    constexpr auto is_empty() const -> bool {
        return flags == None && fg.is_empty() && bg.is_empty();
    }

    // ----- output -----

    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_sgr_params(OutputIt out) const -> OutputIt {
        using namespace detail;
        if (is_empty()) return out;
        bool needs_separate = false;

        if (fg) {
            out = fg.to_sgr_params(out, false);
            needs_separate = true;
        }
        if (bg) {
            if (needs_separate) *out++ = ';';
            out = bg.to_sgr_params(out, true);
            needs_separate = true;
        }

        if (!flags) return out;
        uint8_t current_flag = flags;
        int count = 0;
        do {
            if (current_flag & 1) {
                if (needs_separate) *out++ = ';';
                out = write_to(out, SGR_PARAM_STYLE[count]);
                needs_separate = true;
            }
            count++;
            current_flag >>= 1;
        } while (current_flag);
        return out;
    }

    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out) const -> OutputIt {
        out = detail::write_to(out, "\x1b[");
        out = to_sgr_params(out);
        *out++ = 'm';
        return out;
    }

    [[nodiscard]]
    auto to_escape() const -> std::string {
        std::string esc;
        to_escape(std::back_inserter(esc));
        return esc;
    }
};

struct AbsoluteStyle {
    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE = 
        Style::MAX_ESCAPE_CODE_SIZE + 1;

    Style style;

    constexpr explicit AbsoluteStyle(Style style = Style()) : style(style) {}

    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out) const -> OutputIt {
        out = detail::write_to(out, "\x1b[;");
        out = style.to_sgr_params(out);
        *out++ = 'm';
        return out;
    }
    
    [[nodiscard]]
    auto to_escape() const -> std::string {
        std::string esc;
        to_escape(std::back_inserter(esc));
        return esc;
    }
};

inline constexpr auto absolute(Style style = Style()) -> AbsoluteStyle {
    return AbsoluteStyle(style);
}

inline constexpr auto color(Color fg, Color bg) -> Style {
    return Style(Style::None, fg, bg);
}

inline constexpr auto fg(Color fg) -> Style {
    return Style(Style::None, fg);
}

inline constexpr auto bg(Color bg) -> Style { 
    return Style(Style::None, null_color, bg);
}

// ----- style constants -----

inline constexpr Style bold              = Style(Style::Bold);
inline constexpr Style dim               = Style(Style::Dim);
inline constexpr Style italic            = Style(Style::Italic);
inline constexpr Style underline         = Style(Style::Underline);
inline constexpr Style blink             = Style(Style::Blink);
inline constexpr Style invert            = Style(Style::Invert);
inline constexpr Style strikethrough     = Style(Style::Strikethrough);
inline constexpr Style underline_double  = Style(Style::UnderlineDouble);

// ╔═════════════════════════════════════════════════════════╗
// ║                         output                          ║
// ╚═════════════════════════════════════════════════════════╝

namespace detail {

class OutputState {
public:
    OutputState() = default;

    // ----- options -----

    auto style_enabled() const -> bool { return style_enabled_; }
    auto style_stack_enabled() const -> bool { return style_stack_enabled_; }
    auto color_fallback_enabled() const -> bool {
        return color_fallback_enabled_;
    }

    void set_style(bool enable) { style_enabled_ = enable; }
    void set_style_stack(bool enable) { style_stack_enabled_ = enable; }
    void set_color_fallback(bool enable) { color_fallback_enabled_ = enable; }

    // ----- style operations -----

    auto current_style() const -> AbsoluteStyle {
        if (!style_stack_enabled_ || style_stack_.empty()) return current_;
        else return style_stack_.back();
    }

    void push_style(Style style) {
        AbsoluteStyle new_style = absolute(current_style().style | style);
        if (!style_stack_enabled_) current_ = new_style;
        else style_stack_.push_back(new_style);
    }

    void push_style(AbsoluteStyle style) {
        if (!style_stack_enabled_) current_ = style;
        else style_stack_.push_back(style);
    }

    void pop_style() {
        if (style_stack_enabled_ && !style_stack_.empty()) 
            style_stack_.pop_back();
        else current_ = absolute();
    }

    void reset_style_stack() {
        if (style_stack_enabled_) style_stack_.clear();
        else current_ = absolute();
    }

private:
    std::vector<AbsoluteStyle> style_stack_;
    AbsoluteStyle current_ = absolute();     // if style stack is not enabled

    bool style_enabled_ = true;
    bool style_stack_enabled_ = false;
    bool color_fallback_enabled_ = false;
};

inline auto output_state() -> OutputState& {
    static OutputState instance;
    return instance;
}

}   // namespace detail

struct style_pop_t {};
struct style_reset_t {};

inline constexpr style_pop_t pop{};
inline constexpr style_reset_t reset{};

// ----- ostream operators -----

/// @brief ostream operator for StyleTypes.
template <detail::OutputableStyle StyleT>
inline auto operator<<(std::ostream& os, StyleT rhs) -> std::ostream& {
    using namespace detail;
    auto output = output_state();

    output.push_style(rhs);
    if (!output.style_enabled()) return os;
    rhs.to_escape(std::ostreambuf_iterator<char>(os));
    return os;
}

/// @brief ostream operator for deco::pop.
inline auto operator<<(std::ostream& os, style_pop_t) -> std::ostream& {
    using namespace detail;
    auto output = output_state();

    output.pop_style();
    if (!output.style_enabled()) return os;
    auto current = output.current_style();
    current.to_escape(std::ostreambuf_iterator<char>(os));
    return os;
}

/// @brief ostream operator for deco::reset.
inline auto operator<<(std::ostream& os, style_reset_t) -> std::ostream& {
    using namespace detail;
    auto output = output_state();

    output.reset_style_stack();
    if (!output.style_enabled()) return os;
    AbsoluteStyle().to_escape(std::ostreambuf_iterator<char>(os));
    return os;
}

// ----- style output options -----

inline void set_style_output(bool enable) { 
    detail::output_state().set_style(enable);
}

inline void set_style_stack(bool enable) { 
    detail::output_state().set_style_stack(enable);
}

}   // namespace deco

#endif  // !DECOTERM_DECOTERM_HPP
