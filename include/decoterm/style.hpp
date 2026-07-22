// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_STYLE_HPP
#define DECOTERM_STYLE_HPP

#include <array>
#include <cassert>
#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <version>

#if defined(__cpp_lib_constexpr_cmath) && __cpp_lib_constexpr_cmath >= 202202L
#define DECO_CONSTEXPR_CMATH constexpr
#else
#define DECO_CONSTEXPR_CMATH
#endif

namespace deco {

struct Style;

enum class ColorType : uint8_t { Null = 0, Default, AnsiColor, TrueColor };
using ColorData = std::array<uint8_t, 3>;

namespace detail {

template <typename T>
concept outputable_style = requires(T style, char* out) {
    { style.to_escape(out) } -> std::same_as<char*>;
};

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
    for (auto c : sv)
        *out++ = c;
    return out;
}

template <std::output_iterator<const char&> OutputIt>
inline constexpr auto write_converted_to(OutputIt out, uint8_t value) {
    std::array<char, 3> buf{};
    auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + 3, value);
    assert(ec == std::errc {});
    auto len = ptr - buf.data();
    for (int i = 0; i < len; ++i)
        *out++ = buf[i];
    return out;
}

template <std::output_iterator<const char&> OutputIt>
inline constexpr auto
color_to_sgr_params(OutputIt out, bool is_bg, ColorType type, ColorData data)
    -> OutputIt {
    switch (type) {
    case ColorType::Null:
        return out;
    case ColorType::Default: {
        if (is_bg) out = write_to(out, "49");
        else out = write_to(out, "39");
        return out;
    }
    case ColorType::AnsiColor: {
        uint8_t index = data[0];
        if (index < 16) {
            out = write_to(out,
                           is_bg ? SGR_PARAM_BG[index] : SGR_PARAM_FG[index]);
        } else {
            out = write_to(out, is_bg ? "48;5;" : "38;5;");
            out = write_converted_to(out, index);
        }
        return out;
    }
    case ColorType::TrueColor: {
        auto [r, g, b] = data;
        out = write_to(out, is_bg ? "48;2;" : "38;2;");
        out = write_converted_to(out, r);
        *out++ = ';';
        out = write_converted_to(out, g);
        *out++ = ';';
        out = write_converted_to(out, b);
        return out;
    }
    default:
        return out;
    }
    return out;
}

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                          Color                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A class representing a terminal color.
struct Color {
    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE = 19;

    // ----- constructors -----

    static constexpr auto default_color() -> Color {
        return Color(ColorType::Default, {0, 0, 0});
    }

    static constexpr auto null_color() -> Color {
        return Color(ColorType::Null, {0, 0, 0});
    }

    /// @brief Create a color from index [0, 255]
    constexpr explicit Color(uint8_t index)
        : type_(ColorType::AnsiColor),
          data_({index, 0, 0}) {}

    constexpr explicit Color(uint8_t r, uint8_t g, uint8_t b)
        : type_(ColorType::TrueColor),
          data_({r, g, b}) {}

    constexpr auto operator==(const Color&) const -> bool = default;
    constexpr auto operator!=(const Color&) const -> bool = default;

    constexpr auto type() const -> ColorType { return type_; }

    [[nodiscard]]
    auto to_escape(bool is_bg) const -> std::string {
        std::string esc = "\x1b[";
        detail::color_to_sgr_params(
            std::back_inserter(esc), is_bg, type_, data_);
        esc.push_back('m');
        return esc;
    }

    [[nodiscard]]
    auto debug_string() const -> std::string {
        switch (type_) {
        case ColorType::Null:
            return "[null color]";
        case ColorType::Default:
            return "[Default]";
        case ColorType::AnsiColor: {
            std::string debug("[type=AnsiColor");
            return debug.append(", index=")
                .append(std::to_string(data_[0]))
                .append("]");
        }
        case ColorType::TrueColor: {
            std::string debug("[type=TrueColor");
            return debug.append(", rgb=")
                .append(std::to_string(data_[0]))
                .append(", ")
                .append(std::to_string(data_[1]))
                .append(", ")
                .append(std::to_string(data_[2]))
                .append("]");
        }
        }
    }

  private:
    friend struct Style;

    constexpr Color(ColorType type, ColorData data)
        : type_(type),
          data_(data) {}

    ColorType type_;
    ColorData data_;
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
        throw std::invalid_argument("deco::rgb(): rgb > 0xffffff");
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
inline DECO_CONSTEXPR_CMATH auto hsv(uint16_t h, uint8_t s, uint8_t v) -> Color {  //NOLINT
    // clang-format off
    // NOLINTBEGIN
    if (h < 0 || h >= 360) throw std::invalid_argument(
        "deco::hsv(): h is not in range [0, 360)");

    float hp = h / 60.f;
    float sp = s / 255.0f;
    float vp = v / 255.0f;

    float f = hp - std::floor(hp);
    uint8_t p = std::round(vp * (1 - sp) * 255);
    uint8_t q = std::round(vp * (1 - (f * sp)) * 255);
    uint8_t t = std::round(vp * (1 - ((1 - f) * sp)) * 255);

    switch (h / 60) {
        case 0 : return Color(v, t, p);
        case 1 : return Color(q, v, p);
        case 2 : return Color(p, v, t);
        case 3 : return Color(p, q, v);
        case 4 : return Color(t, p, v);
        case 5 : return Color(v, p, q);
        default: assert(false);
    }
    // NOLINTEND
    // clang-format on
}

namespace colors {

// clang-format off

/// system colors
enum Color16 : uint8_t {    // NOLINT
    Black               = 0,
    Red                 = 1,
    Green               = 2,
    Yellow              = 3,
    Blue                = 4,
    Magenta             = 5,
    Cyan                = 6,
    White               = 7,
    BrightBlack         = 8,
    BrightRed           = 9,
    BrightGreen         = 10,
    BrightYellow        = 11,
    BrightBlue          = 12,
    BrightMagenta       = 13,
    BrightCyan          = 14,
    BrightWhite         = 15,
};

}   // namespace colors

/* ----- color constants ----- */

inline constexpr Color default_color   = Color::default_color();
inline constexpr Color null_color      = Color::null_color();

inline constexpr Color black           = Color(colors::Black);
inline constexpr Color red             = Color(colors::Red);
inline constexpr Color green           = Color(colors::Green);
inline constexpr Color yellow          = Color(colors::Yellow);
inline constexpr Color blue            = Color(colors::Blue);
inline constexpr Color magenta         = Color(colors::Magenta);
inline constexpr Color cyan            = Color(colors::Cyan);
inline constexpr Color white           = Color(colors::White);
inline constexpr Color bright_black    = Color(colors::BrightBlack);
inline constexpr Color bright_red      = Color(colors::BrightRed);
inline constexpr Color bright_green    = Color(colors::BrightGreen);
inline constexpr Color bright_yellow   = Color(colors::BrightYellow);
inline constexpr Color bright_blue     = Color(colors::BrightBlue);
inline constexpr Color bright_magenta  = Color(colors::BrightMagenta);
inline constexpr Color bright_cyan     = Color(colors::BrightCyan);
inline constexpr Color bright_white    = Color(colors::BrightWhite);

// clang-format on

// ╔═════════════════════════════════════════════════════════╗
// ║                          Style                          ║
// ╚═════════════════════════════════════════════════════════╝

struct style_reset_t {};
inline constexpr style_reset_t reset;

/// @brief A class representing a terminal style.
struct Style {
    // clang-format off
    enum Flags : uint8_t {      //NOLINT
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
        (Color::MAX_ESCAPE_CODE_SIZE * 2) + 13 + 3;

    // clang-format on

    /// @brief create a default style (flags = 0, fg = null_color, bg =
    /// null_color)
    constexpr Style() = default;

    constexpr Style(uint8_t flags, Color fg = null_color, Color bg = null_color)
        : fg_data_(fg.data_),
          bg_data_(bg.data_),
          color_types_((static_cast<uint8_t>(fg.type_) << 4)
                       | static_cast<uint8_t>(bg.type_)),
          flags_(flags) {}

    // ----- operators -----

    /// @brief combine two styles, with rhs taking precedence
    constexpr auto operator|(Style rhs) const -> Style {
        Style combined = *this;
        combined.flags_ |= rhs.flags_;
        if (!rhs.fg_null()) {
            combined.fg_data_ = rhs.fg_data_;
            combined.set_fg_type(rhs.fg_type());
        }
        if (!rhs.bg_null()) {
            combined.bg_data_ = rhs.bg_data_;
            combined.set_bg_type(rhs.bg_type());
        }
        return combined;
    }

    constexpr void operator|=(Style rhs) { *this = *this | rhs; }

    constexpr auto operator==(const Style&) const -> bool = default;
    constexpr auto operator!=(const Style&) const -> bool = default;

    /// @brief equivalent to !empty()
    constexpr explicit operator bool() const { return !empty(); }

    // ----- observe -----

    constexpr auto flags() const -> uint8_t { return flags_; }
    constexpr auto fg() const -> Color { return {fg_type(), fg_data_}; }
    constexpr auto bg() const -> Color { return {bg_type(), bg_data_}; }

    constexpr auto empty() const -> bool {
        return flags_ == None && fg_null() && bg_null();
    }

    // ----- output -----

    /// @brief write SGR parameters to output iterator
    /// @details format: "P1;P2;...;Pn"
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_sgr_params(OutputIt out) const -> OutputIt {
        using namespace detail;
        if (empty()) return out;
        bool needs_separate = false;

        if (!fg_null()) {
            out = detail::color_to_sgr_params(out, false, fg_type(), fg_data_);
            needs_separate = true;
        }
        if (!bg_null()) {
            if (needs_separate) *out++ = ';';
            out = detail::color_to_sgr_params(out, true, bg_type(), bg_data_);
            needs_separate = true;
        }

        if (!flags_) return out;
        uint8_t current_flag = flags_;
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

    /// @brief write ANSI escape code to output iterator
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out) const -> OutputIt {
        out = detail::write_to(out, "\x1b[");
        out = to_sgr_params(out);
        *out++ = 'm';
        return out;
    }

    /// @brief write ANSI escape code to string
    [[nodiscard]]
    auto to_escape() const -> std::string {
        std::string esc;
        to_escape(std::back_inserter(esc));
        return esc;
    }

    [[nodiscard]]
    auto debug_string() const -> std::string {
        std::string debug = "[flags=";
        std::array<char, 8> buf{};
        for (std::size_t i = 0; i < buf.size(); ++i) {
            uint8_t bit = 1 << (buf.size() - i - 1);
            buf[i] = (flags_ & bit) ? '1' : '0';
        }
        return debug.append(buf.data(), 8)
            .append(", fg=")
            .append(fg().debug_string())
            .append(", bg=")
            .append(bg().debug_string())
            .append("]");
    }

  private:
    constexpr auto fg_type() const -> ColorType {
        return static_cast<ColorType>((color_types_ >> 4) & 0x0F);
    }

    constexpr auto bg_type() const -> ColorType {
        return static_cast<ColorType>(color_types_ & 0x0F);
    }

    constexpr void set_fg_type(ColorType type) {
        color_types_ =
            (static_cast<uint8_t>(type) << 4) | (color_types_ & 0x0F);
    }

    constexpr void set_bg_type(ColorType type) {
        color_types_ =
            (static_cast<uint8_t>(type) & 0x0F) | (color_types_ & 0xF0);
    }

    constexpr auto fg_null() const -> bool {
        return fg_type() == ColorType::Null;
    }
    constexpr auto bg_null() const -> bool {
        return bg_type() == ColorType::Null;
    }

    ColorData fg_data_ = {0, 0, 0};
    ColorData bg_data_ = {0, 0, 0};

    // bit-packed color types for fg and bg.
    // fg uses upper 4 bits and bg uses lower 4 bits.
    uint8_t color_types_ = 0;

    uint8_t flags_ = None;
};

/// @brief A Style wrapper for representing an absolute style
/// (not relative to the current style).
struct AbsoluteStyle {
    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE =
        Style::MAX_ESCAPE_CODE_SIZE + 1;

    Style style;

    constexpr explicit AbsoluteStyle(Style style = Style()) : style(style) {}

    constexpr auto operator==(const AbsoluteStyle&) const -> bool = default;
    constexpr auto operator!=(const AbsoluteStyle&) const -> bool = default;

    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out) const -> OutputIt {
        out = detail::write_to(out, "\x1b[");
        if (style) *out++ = ';';
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

/* ----- helper functions ----- */

/// @brief create an AbsoluteStyle from a Style
inline constexpr auto absolute(Style style) -> AbsoluteStyle {
    return AbsoluteStyle(style);
}

/// @brief create a Style with foreground and background colors
inline constexpr auto color(Color fg, Color bg) -> Style {
    return {Style::None, fg, bg};
}

/// @brief create a Style with foreground color
inline constexpr auto fg(Color fg) -> Style { return {Style::None, fg}; }

/// @brief create a Style with background color
inline constexpr auto bg(Color bg) -> Style {
    return {Style::None, null_color, bg};
}

/* ----- ostream operators ----- */

/// @brief output operator for Style types e.g. Style, AbsoluteStyle
template <detail::outputable_style StyleT>
inline auto operator<<(std::ostream& os, StyleT style) -> std::ostream& {
    style.to_escape(std::ostreambuf_iterator(os));
    return os;
}

/// @brief output operator for deco::reset
inline auto operator<<(std::ostream& os, style_reset_t) -> std::ostream& {
    absolute(Style()).to_escape(std::ostreambuf_iterator(os));
    return os;
}

/* ----- style constants ----- */

// clang-format off

inline constexpr Style default_style     = Style();
inline constexpr Style bold              = Style(Style::Bold);
inline constexpr Style dim               = Style(Style::Dim);
inline constexpr Style italic            = Style(Style::Italic);
inline constexpr Style underline         = Style(Style::Underline);
inline constexpr Style blink             = Style(Style::Blink);
inline constexpr Style invert            = Style(Style::Invert);
inline constexpr Style strikethrough     = Style(Style::Strikethrough);
inline constexpr Style underline_double  = Style(Style::UnderlineDouble);

// clang-format on

} // namespace deco

#endif // !DECOTERM_STYLE_HPP
