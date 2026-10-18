// ┌                                                         ┐
// │        THIS FILE IS AUTO GENERATED, DO NOT EDIT!        │
// └                                                         ┘

// C++20 Terminal styling library
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
#include <type_traits>
#include <version>

#if defined(__cpp_lib_constexpr_cmath) && __cpp_lib_constexpr_cmath >= 202202L
#define DECO_CONSTEXPR_CMATH constexpr
#else
#define DECO_CONSTEXPR_CMATH
#endif

namespace deco {

struct Style;
struct AbsoluteStyle;

/// @see Color
enum class ColorType : uint8_t {
    Null = 0,
    DefaultColor,
    TerminalColor,
    TrueColor,
};

template <typename T>
concept ostream_outputable = requires(std::ostream& os, const T& value) {
    { os << value } -> std::same_as<std::ostream&>;
};

namespace detail {

// clang-format off

/// Whether remove_cvref_t<StyleT> is a Style or an AbsoluteStyle.
/// The requires expression describes the common interface of both types.
template <typename S>
concept style_type = (std::is_same_v<std::remove_cvref_t<S>, Style>
        || std::is_same_v<std::remove_cvref_t<S>, AbsoluteStyle>)
    && requires(
        const std::remove_cvref_t<S>& style,
        const std::remove_cvref_t<S>& other_style,
        char* out
    ) {
        { style.is_null() } -> std::same_as<bool>;
        { style == other_style } -> std::same_as<bool>;
        { style != other_style } -> std::same_as<bool>;
        { style.to_escape(out) } -> std::same_as<char*>;
    };

// clang-format on

using ColorData = std::array<uint8_t, 3>;

// clang-format off
inline constexpr std::array<std::string_view, 16> SGR_PARAM_FG {
    "30", "31", "32",
    "33", "34", "35",
    "36", "37", "90",
    "91", "92", "93",
    "94", "95", "96",
    "97",
};

inline constexpr std::array<std::string_view, 16> SGR_PARAM_BG {
    "40", "41", "42",
    "43", "44", "45",
    "46", "47", "100",
    "101", "102", "103",
    "104", "105", "106",
    "107",
};

inline constexpr std::array<std::string_view, 8> SGR_PARAM_STYLE {
    "1" /*bold*/,           "2"  /*dim*/,           "3" /*italic*/,
    "4" /*underline*/,      "5"  /*blink*/,         "7" /*invert*/,
    "9" /*strikethrough*/,  "21" /*double underline*/,
};
// clang-format on

template <std::output_iterator<const char&> OutputIt>
constexpr auto write_to(OutputIt out, std::string_view sv) -> OutputIt {
    for (auto c : sv)
        *out++ = c;
    return out;
}

template <std::output_iterator<const char&> OutputIt>
constexpr auto write_to(OutputIt out, uint8_t value) {
    std::array<char, 3> buf {};
    const auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + 3, value);
    assert(ec == std::errc {});
    for (int i = 0; i < ptr - buf.data(); ++i)
        *out++ = buf[i];
    return out;
}

template <bool BG, std::output_iterator<const char&> OutputIt>
constexpr auto color_to_sgr_params(OutputIt out, ColorType type, ColorData data)
    -> OutputIt {
    switch (type) {
        case ColorType::Null:
            return out;
        case ColorType::DefaultColor: {
            if constexpr (BG) return write_to(out, "49");
            else return write_to(out, "39");
        }
        case ColorType::TerminalColor: {
            uint8_t index = data[0];
            if (index < 16) {
                if constexpr (BG) return write_to(out, SGR_PARAM_BG[index]);
                else return write_to(out, SGR_PARAM_FG[index]);
            } else {
                if constexpr (BG) out = write_to(out, "48;5;");
                else out = write_to(out, "38;5;");
                out = write_to(out, index);
            }
            return out;
        }
        case ColorType::TrueColor: {
            auto [r, g, b] = data;
            if constexpr (BG) out = write_to(out, "48;2;");
            else out = write_to(out, "38;2;");
            out = write_to(out, r);
            *out++ = ';';
            out = write_to(out, g);
            *out++ = ';';
            out = write_to(out, b);
            return out;
        }
    }
    assert(false);
    return out; // unreachable
}

template <style_type StyleT, typename T>
struct Styled {
    using value_type = T;

    constexpr Styled(const T& value, StyleT style)
        : value_(value),
          style_(style) {}

    constexpr auto value() const -> const T& { return value_; }
    constexpr auto style() const -> StyleT { return style_; }

  private:
    const T& value_; // NOLINT
    StyleT style_;
};

template <typename T> struct is_styled : std::false_type {};

template <typename S, typename T>
struct is_styled<Styled<S, T>> : std::true_type {};

template <typename T>
concept styled = is_styled<std::remove_cvref_t<T>>::value;

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                          Color                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Represents a nullable terminal color.
///
/// @details
/// A `Color` is similar to a tagged union with 4 color types.
/// It stores a `ColorType` and a `uint8_t[3]` containing the data.
///
/// Color types:
/// * `Null` => A null color. It emits no ANSI escape sequence.
///
/// * `DefaultColor` => The terminal's default color.
///
/// * `TerminalColor` => System 16 color or XTerm 256 color.
///     `data[0]` stores the index [0, 255], with indices 0-15 corresponding
///     to the system 16 colors.
///     (See https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit)
///
/// * `TrueColor` => A 24-bit RGB color.
///     `data[0]`, `data[1]`, `data[2]` store the red, green and blue components
///     respectively.
struct Color {
    static constexpr std::size_t MAX_ESCAPE_SEQ_SIZE = 19;

    /* ----- constructors ----- */

    static constexpr auto default_color() -> Color {
        return Color(ColorType::DefaultColor, {0, 0, 0});
    }

    static constexpr auto null_color() -> Color {
        return Color(ColorType::Null, {0, 0, 0});
    }

    /// Create a color implicitly from an index in [0, 255]
    constexpr Color(uint8_t index)
        : type_(ColorType::TerminalColor),
          data_({index, 0, 0}) {}

    constexpr explicit Color(uint8_t r, uint8_t g, uint8_t b)
        : type_(ColorType::TrueColor),
          data_({r, g, b}) {}

    /* ----- observe ----- */

    constexpr auto operator==(const Color&) const -> bool = default;
    constexpr auto operator!=(const Color&) const -> bool = default;

    /// equivalent to `!is_null()`
    constexpr explicit operator bool() const { return !this->is_null(); }

    /// Whether this Color's type is `ColorType::null`
    constexpr auto is_null() const -> bool { return type_ == ColorType::Null; }
    constexpr auto type() const -> ColorType { return type_; }

    /// @brief Returns the raw data(`uint8_t[3]`) stored in this Color.
    ///
    /// @details
    /// Usage:
    /// ```cpp
    /// Color col = rgb(32, 64, 128);
    /// if (col.type() == ColorType::TrueColor) {
    ///     auto [r, g, b] = col.data();
    ///     /* ... */
    /// } else if (col.type() == ColorType::TerminalColor) {
    ///     uint8_t index = col.data()[0];
    ///     /* ... */
    /// }
    /// ```
    constexpr auto data() const -> const detail::ColorData& { return data_; }

    /* ----- output ----- */

    [[nodiscard]] auto to_escape_fg() const -> std::string {
        std::string esc = "\x1b[";
        detail::color_to_sgr_params<false>(
            std::back_inserter(esc), type_, data_);
        esc.push_back('m');
        return esc;
    }

    [[nodiscard]] auto to_escape_bg() const -> std::string {
        std::string esc = "\x1b[";
        detail::color_to_sgr_params<true>(
            std::back_inserter(esc), type_, data_);
        esc.push_back('m');
        return esc;
    }

    [[nodiscard]] auto debug_string() const -> std::string {
        switch (type_) {
            case ColorType::Null:
                return "[null_color]";
            case ColorType::DefaultColor:
                return "[default_color]";
            case ColorType::TerminalColor: {
                return std::string("[terminal_color: ")
                    .append(std::to_string(data_[0]))
                    .append("]");
            }
            case ColorType::TrueColor: {
                return std::string("[true_color: ")
                    .append(std::to_string(data_[0]))
                    .append(", ")
                    .append(std::to_string(data_[1]))
                    .append(", ")
                    .append(std::to_string(data_[2]))
                    .append("]");
            }
        }
        assert(false);
        return ""; // unreachable
    }

  private:
    friend struct Style;

    constexpr Color(ColorType type, detail::ColorData data)
        : type_(type),
          data_(data) {}

    ColorType type_;
    detail::ColorData data_;
};

/// Creates a color from RGB.
constexpr auto rgb(uint8_t r, uint8_t g, uint8_t b) -> Color {
    return Color(r, g, b);
}

/// @brief Creates a color from 0xRRGGBB.
///
/// @pre rgb <= 0xFFFFFF
constexpr auto rgb(uint32_t hex) -> Color {
    // clang-format off
    if (hex > 0xffffff) 
        throw std::invalid_argument("deco::rgb(): rgb > 0xffffff");
    return Color((hex >> 16u) & 0xFFu,
                    (hex >> 8u) & 0xFFu,
                    hex & 0xFFu);
    // clang-format on
}

/// @brief Creates a color from HSV.
///
/// @param h [0, 360): Hue of the color
/// @param s [0, 255]: Saturation of the color
/// @param v [0, 255]: Value (brightness) of the color
[[nodiscard]]
inline DECO_CONSTEXPR_CMATH auto hsv(uint16_t h, uint8_t s, uint8_t v) // NOLINT
    -> Color {
    // clang-format off

    if (h >= 360) throw std::invalid_argument(
        "deco::hsv(): h is not in range [0, 360)");

    float hp = h / 60.f;                               //NOLINT
    float sp = s / 255.0f;                             //NOLINT
    float vp = v / 255.0f;                             //NOLINT

    float f = hp - std::floor(hp);
    uint8_t p = static_cast<uint8_t>(std::round(vp * (1 - sp) * 255));             //NOLINT
    uint8_t q = static_cast<uint8_t>(std::round(vp * (1 - (f * sp)) * 255));       //NOLINT
    uint8_t t = static_cast<uint8_t>(std::round(vp * (1 - ((1 - f) * sp)) * 255)); //NOLINT

    switch (h / 60) {
        case 0 : return Color(v, t, p);
        case 1 : return Color(q, v, p);
        case 2 : return Color(p, v, t);
        case 3 : return Color(p, q, v);
        case 4 : return Color(t, p, v);
        case 5 : return Color(v, p, q);
        default: assert(false);
    }

    return Color::null_color();
    // clang-format on 
}

/* ----- color constants ----- */

inline constexpr Color default_color   = Color::default_color();
inline constexpr Color null_color      = Color::null_color();

inline constexpr Color black           = Color(0);
inline constexpr Color red             = Color(1);
inline constexpr Color green           = Color(2);
inline constexpr Color yellow          = Color(3);
inline constexpr Color blue            = Color(4);
inline constexpr Color magenta         = Color(5);
inline constexpr Color cyan            = Color(6);
inline constexpr Color white           = Color(7);
inline constexpr Color bright_black    = Color(8);
inline constexpr Color bright_red      = Color(9);
inline constexpr Color bright_green    = Color(10);
inline constexpr Color bright_yellow   = Color(11);
inline constexpr Color bright_blue     = Color(12);
inline constexpr Color bright_magenta  = Color(13);
inline constexpr Color bright_cyan     = Color(14);
inline constexpr Color bright_white    = Color(15);

// clang-format on

// ╔═════════════════════════════════════════════════════════╗
// ║                          Style                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Represents a nullable terminal style.
///
/// @details
/// * It contains 8 emphasis flags and 2 `Color`s for foreground color and
/// background color.
///
/// * A null 'Style' emits no ANSI escape sequence.
struct Style {
    // clang-format off
    enum Emphasis : uint8_t {                              //NOLINT
        None                = 0,
        Bold                = 1u << 0u,
        Dim                 = 1u << 1u,
        Italic              = 1u << 2u,
        Underline           = 1u << 3u,
        Blink               = 1u << 4u,
        Invert              = 1u << 5u,
        Strikethrough       = 1u << 6u,
        UnderlineDouble     = 1u << 7u,
    };

    static constexpr std::size_t MAX_ESCAPE_SEQ_SIZE =
        ((Color::MAX_ESCAPE_SEQ_SIZE - 3 + 1) * 2) + 16 + 3;

    // clang-format on

    explicit constexpr Style(uint8_t emphasis = 0,
                             Color fg = null_color,
                             Color bg = null_color)
        : fg_data_(fg.data()),
          bg_data_(bg.data()),
          color_types_((static_cast<unsigned int>(fg.type()) << 4u)
                       | static_cast<unsigned int>(bg.type())),
          emphasis_(emphasis) {}

    // ----- operators -----

    /// @brief Combines two styles.
    ///
    /// @details
    /// It works like applying 2 ANSI escape sequences, meaning that
    /// `std::cout << style1 << style2` is roughly equivalent to
    /// `std::cout << (style1 | style2)`.
    ///
    /// For `lhs | rhs`:
    /// * Do `OR` operation of both emphasis.
    /// * Each non-null color in `rhs` overrides the corresponding color in
    ///  `lhs`.
    ///
    /// Examples:
    /// (italic, fg=red) | (bold) = (italic | bold, fg=red)
    /// (fg=black, bg=blue) | (bg=green) = (fg=black, bg=green)
    constexpr auto operator|(Style rhs) const -> Style {
        Style combined = *this;
        combined.emphasis_ |= rhs.emphasis_;
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

    /// equivalent to !is_null()
    constexpr explicit operator bool() const { return !this->is_null(); }

    // ----- observe -----

    constexpr auto emphasis() const -> uint8_t { return emphasis_; }
    constexpr auto fg() const -> Color { return {this->fg_type(), fg_data_}; }
    constexpr auto bg() const -> Color { return {this->bg_type(), bg_data_}; }

    /// Will `to_escape()` return a empty string?
    constexpr auto is_null() const -> bool {
        return emphasis_ == None && this->fg_null() && this->bg_null();
    }

    // ----- output -----

    /// Writes SGR parameters to the output iterator in the form "P1;P2;...;Pn".
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_sgr_params(OutputIt out) const -> OutputIt {
        using namespace detail;
        bool needs_separate = false;

        if (!this->fg_null()) {
            out = color_to_sgr_params<false>(out, this->fg_type(), fg_data_);
            needs_separate = true;
        }
        if (!this->bg_null()) {
            if (needs_separate) *out++ = ';';
            out = color_to_sgr_params<true>(out, this->bg_type(), bg_data_);
            needs_separate = true;
        }

        uint8_t current_flag = emphasis_;
        int count = 0;
        do {
            if (current_flag & 1u) {
                if (needs_separate) *out++ = ';';
                out = write_to(out, SGR_PARAM_STYLE[count]);
                needs_separate = true;
            }
            count++;
            current_flag >>= 1u;
        } while (current_flag);
        return out;
    }

    /// Writes ANSI escape sequence to the output iterator.
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out) const -> OutputIt {
        if (this->is_null()) return out;

        out = detail::write_to(out, "\x1b[");
        out = this->to_sgr_params(out);
        *out++ = 'm';
        return out;
    }

    /// Writes ANSI escape sequence to a string.
    [[nodiscard]] auto to_escape() const -> std::string {
        std::string esc;
        this->to_escape(std::back_inserter(esc));
        return esc;
    }

    [[nodiscard]] auto debug_string() const -> std::string {
        std::string debug = "[flags=";
        std::array<char, 8> buf {};

        for (unsigned int i = 0; i < 8; i++) {
            if ((static_cast<unsigned int>(emphasis_) >> (7u - i)) & 1u)
                buf[i] = '1';
            else buf[i] = '0';
        }

        return debug.append(buf.data(), 8)
            .append(", fg=")
            .append(this->fg().debug_string())
            .append(", bg=")
            .append(this->bg().debug_string())
            .append("]");
    }

  private:
    constexpr auto fg_type() const -> ColorType {
        return static_cast<ColorType>(
            (static_cast<unsigned int>(color_types_) >> 4u) & 0x0Fu);
    }

    constexpr auto bg_type() const -> ColorType {
        return static_cast<ColorType>(color_types_ & 0x0Fu);
    }

    constexpr void set_fg_type(ColorType type) {
        color_types_ =
            (static_cast<unsigned int>(type) << 4u) | (color_types_ & 0x0Fu);
    }

    constexpr void set_bg_type(ColorType type) {
        color_types_ =
            (static_cast<unsigned int>(type) & 0x0Fu) | (color_types_ & 0xF0u);
    }

    constexpr auto fg_null() const -> bool {
        return this->fg_type() == ColorType::Null;
    }
    constexpr auto bg_null() const -> bool {
        return this->bg_type() == ColorType::Null;
    }

    detail::ColorData fg_data_ = {0, 0, 0};
    detail::ColorData bg_data_ = {0, 0, 0};

    // Bit-packed color types for fg and bg.
    // fg uses the upper 4 bits and bg uses the lower 4 bits.
    uint8_t color_types_ = 0;

    uint8_t emphasis_ = None;
};

/// @brief A Style wrapper that represents an absolute style.
///
/// @details
/// Style output normally has *additive semantics*, but this does't have.
///
/// Example for `std::cout`:
/// ```cpp
/// std::cout << italic << bold; // current style: italic | bold
/// std::cout << reset;
/// std::cout << italic << abs(bold); // current style: bold
/// ```
struct AbsoluteStyle {
    static constexpr std::size_t MAX_ESCAPE_SEQ_SIZE =
        Style::MAX_ESCAPE_SEQ_SIZE + 1;

    constexpr explicit AbsoluteStyle(Style style = Style()) : style_(style) {}

    constexpr auto operator==(const AbsoluteStyle&) const -> bool = default;
    constexpr auto operator!=(const AbsoluteStyle&) const -> bool = default;

    constexpr auto inner() const -> Style { return style_; }

    /// This is always `false` since `to_escape()` will never be empty.
    constexpr auto is_null() const -> bool { return false; }

    /// Writes ANSI escape sequence to the output iterator.
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out) const -> OutputIt {
        out = detail::write_to(out, "\x1b[");
        if (style_) *out++ = ';';
        out = style_.to_sgr_params(out);
        *out++ = 'm';
        return out;
    }

    /// Writes ANSI escape sequence to a string.
    [[nodiscard]] auto to_escape() const -> std::string {
        std::string esc;
        this->to_escape(std::back_inserter(esc));
        return esc;
    }

    [[nodiscard]] auto debug_string() const -> std::string {
        return std::string("[absolute:") + style_.debug_string() + "]";
    }

  private:
    Style style_;
};

/// @brief Creates an AbsoluteStyle from a Style.
///
/// @details equivalent to `AbsoluteStyle(style)`
constexpr auto abs(Style style) -> AbsoluteStyle {
    return AbsoluteStyle(style);
}

/// Creates a Style with foreground and background colors.
constexpr auto color(Color fg, Color bg) -> Style {
    return Style(Style::Emphasis::None, fg, bg);
}

/// Create a Style with foreground color.
constexpr auto fg(Color fg) -> Style {
    return Style(Style::Emphasis::None, fg);
}

/// Create a Style with background color.
constexpr auto bg(Color bg) -> Style {
    return Style(Style::Emphasis::None, null_color, bg);
}

/// IO manipulator that resets the terminal style.
struct Reset {};
inline constexpr Reset reset;

/// output operator for `Style`
inline auto operator<<(std::ostream& os, Style style) -> std::ostream& {
    std::array<char, Style::MAX_ESCAPE_SEQ_SIZE> buf = {0};
    auto len = style.to_escape(buf.begin()) - buf.begin();
    os.write(buf.data(), len);
    return os;
}

/// output operator for `AbsoluteStyle`
inline auto operator<<(std::ostream& os, AbsoluteStyle abstyle)
    -> std::ostream& {
    std::array<char, AbsoluteStyle::MAX_ESCAPE_SEQ_SIZE> buf = {0};
    auto len = abstyle.to_escape(buf.begin()) - buf.begin();
    os.write(buf.data(), len);
    return os;
}

/// output operator for deco::reset
inline auto operator<<(std::ostream& os, Reset) -> std::ostream& {
    std::array<char, Style::MAX_ESCAPE_SEQ_SIZE> buf = {0};
    auto len = abs(Style()).to_escape(buf.begin()) - buf.begin();
    os.write(buf.data(), len);
    return os;
}

/* ----- style constants ----- */

// clang-format off

inline constexpr Style null_style        = Style();
inline constexpr Style bold              = Style(Style::Emphasis::Bold);
inline constexpr Style dim               = Style(Style::Emphasis::Dim);
inline constexpr Style italic            = Style(Style::Emphasis::Italic);
inline constexpr Style underline         = Style(Style::Emphasis::Underline);
inline constexpr Style blink             = Style(Style::Emphasis::Blink);
inline constexpr Style invert            = Style(Style::Emphasis::Invert);
inline constexpr Style strikethrough     = Style(Style::Emphasis::Strikethrough);
inline constexpr Style underline_double  = Style(Style::Emphasis::UnderlineDouble);

// clang-format on

// ╔═════════════════════════════════════════════════════════╗
// ║                         styled                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Creates a styled wrapper aroune a value.
///
/// @details
/// `std::cout << styled(value, style)` is equivalent to
/// `std::cout << style << value << deco::reset;`.
///
/// @warning
/// The returned object has **reference semantics** and must not outlive value.
///
/// **Example**
/// ```cpp
/// auto styled_int = styled(42, bold);  // BAD: dangling reference
/// std::cout << styled(42, bold);       // OK
/// ```
template <typename T>
    requires(!detail::style_type<T>)
constexpr auto styled(const T& value, Style style) {
    return detail::Styled(value, style);
}

/// `AbsoluteStyle` version of `styled()`
template <typename T>
    requires(!detail::style_type<T>)
constexpr auto styled(const T& value, AbsoluteStyle abstyle) {
    return detail::Styled(value, abstyle);
}

/// shorthand for `styled(value, style)`
template <typename T>
    requires(!detail::style_type<T> && !detail::styled<T>)
constexpr auto operator|(const T& value, Style style) {
    return styled(value, style);
}

/// shorthand for `styled(value, abstyle)`
template <typename T>
    requires(!detail::style_type<T> && !detail::styled<T>)
constexpr auto operator|(const T& value, AbsoluteStyle abstyle) {
    return styled(value, abstyle);
}

template <typename T>
constexpr auto operator|(const detail::Styled<Style, T>& styled, Style style)
    -> detail::Styled<Style, T> {
    return detail::Styled(styled.value(), styled.style() | style);
}

/// output operator for `styled()`
template <detail::style_type StyleT, ostream_outputable T>
inline auto operator<<(std::ostream& os,
                       const detail::Styled<StyleT, T>& styled)
    -> std::ostream& {
    os << styled.style() << styled.value() << reset;
    return os;
}

} // namespace deco

#undef DECO_CONSTEXPR_CMATH

#endif // !DECOTERM_STYLE_HPP


#ifndef DECOTERM_COLOR_INFO_HPP
#define DECOTERM_COLOR_INFO_HPP

#include <array>
#include <cstdint>

// ╔═════════════════════════════════════════════════════════╗
// ║                       Color Info                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace deco::colors {

// NOLINTBEGIN
// clang-format off

// ──────────────────────── Terminal color names ─────────────────────

enum TerminalColors : uint8_t {               // NOLINT
    black             = 0,
    red               = 1,
    green             = 2,
    yellow            = 3,
    blue              = 4,
    magenta           = 5,
    cyan              = 6,
    white             = 7,
    bright_black       = 8,
    bright_red         = 9,
    bright_green       = 10,
    bright_yellow      = 11,
    bright_blue        = 12,
    bright_magenta     = 13,
    bright_cyan        = 14,
    bright_white       = 15,

    aquamarine1       = 86,
    aquamarine1_1     = 122,
    aquamarine3       = 79,
    blue1             = 21,
    blue3             = 19,
    blue3_1           = 20,
    blue_violet        = 57,
    cadet_blue         = 72,
    cadet_blue_1       = 73,
    chartreuse1       = 118,
    chartreuse2       = 82,
    chartreuse2_1     = 112,
    chartreuse3       = 70,
    chartreuse3_1     = 76,
    chartreuse4       = 64,
    cornflower_blue    = 69,
    cornsilk1         = 230,
    cyan1             = 51,
    cyan2             = 50,
    cyan3             = 43,
    dark_blue          = 18,
    dark_cyan          = 36,
    dark_goldenrod     = 136,
    dark_green         = 22,
    dark_khaki         = 143,
    dark_magenta       = 90,
    dark_magenta_1     = 91,
    dark_olive_green1   = 191,
    dark_olive_green1_1 = 192,
    dark_olive_green2   = 155,
    dark_olive_green3   = 107,
    dark_olive_green3_1 = 113,
    dark_olive_green3_2 = 149,
    dark_orange        = 208,
    dark_orange3       = 130,
    dark_orange3_1     = 166,
    dark_red           = 52,
    dark_red_1         = 88,
    dark_sea_green      = 108,
    dark_sea_green1     = 158,
    dark_sea_green1_1   = 193,
    dark_sea_green2     = 151,
    dark_sea_green2_1   = 157,
    dark_sea_green3     = 115,
    dark_sea_green3_1   = 150,
    dark_sea_green4     = 65,
    dark_sea_green4_1   = 71,
    dark_slate_gray1    = 123,
    dark_slate_gray2    = 87,
    dark_slate_gray3    = 116,
    dark_turquoise     = 44,
    dark_violet        = 92,
    dark_violet_1      = 128,
    deep_pink1         = 198,
    deep_pink1_1       = 199,
    deep_pink2         = 197,
    deep_pink3         = 161,
    deep_pink3_1       = 162,
    deep_pink4         = 53,
    deep_pink4_1       = 89,
    deep_pink4_2       = 125,
    deep_sky_blue1      = 39,
    deep_sky_blue2      = 38,
    deep_sky_blue3      = 31,
    deep_sky_blue3_1    = 32,
    deep_sky_blue4      = 23,
    deep_sky_blue4_1    = 24,
    deep_sky_blue4_2    = 25,
    dodger_blue1       = 33,
    dodger_blue2       = 27,
    dodger_blue3       = 26,
    gold1             = 220,
    gold3             = 142,
    gold3_1           = 178,
    green1            = 46,
    green3            = 34,
    green3_1          = 40,
    green4            = 28,
    green_yellow       = 154,
    grey0             = 16,
    grey100           = 231,
    grey11            = 234,
    grey15            = 235,
    grey19            = 236,
    grey23            = 237,
    grey27            = 238,
    grey3             = 232,
    grey30            = 239,
    grey35            = 240,
    grey37            = 59,
    grey39            = 241,
    grey42            = 242,
    grey46            = 243,
    grey50            = 244,
    grey53            = 102,
    grey54            = 245,
    grey58            = 246,
    grey62            = 247,
    grey63            = 139,
    grey66            = 248,
    grey69            = 145,
    grey7             = 233,
    grey70            = 249,
    grey74            = 250,
    grey78            = 251,
    grey82            = 252,
    grey84            = 188,
    grey85            = 253,
    grey89            = 254,
    grey93            = 255,
    honeydew2         = 194,
    hot_pink           = 205,
    hot_pink2          = 169,
    hot_pink3          = 132,
    hot_pink3_1        = 168,
    hot_pink_1         = 206,
    indian_red         = 131,
    indian_red1        = 203,
    indian_red1_1      = 204,
    indian_red_1       = 167,
    khaki1            = 228,
    khaki3            = 185,
    light_coral        = 210,
    light_cyan1        = 195,
    light_cyan3        = 152,
    light_goldenrod1   = 227,
    light_goldenrod2   = 186,
    light_goldenrod2_1 = 221,
    light_goldenrod2_2 = 222,
    light_goldenrod3   = 179,
    light_green        = 119,
    light_green_1      = 120,
    light_pink1        = 217,
    light_pink3        = 174,
    light_pink4        = 95,
    light_salmon1      = 216,
    light_salmon3      = 137,
    light_salmon3_1    = 173,
    light_sea_green     = 37,
    light_sky_blue1     = 153,
    light_sky_blue3     = 109,
    light_sky_blue3_1   = 110,
    light_slate_blue    = 105,
    light_slate_grey    = 103,
    light_steel_blue    = 147,
    light_steel_blue1   = 189,
    light_steel_blue3   = 146,
    light_yellow3      = 187,
    magenta1          = 201,
    magenta2          = 165,
    magenta2_1        = 200,
    magenta3          = 127,
    magenta3_1        = 163,
    magenta3_2        = 164,
    medium_orchid      = 134,
    medium_orchid1     = 171,
    medium_orchid1_1   = 207,
    medium_orchid3     = 133,
    medium_purple      = 104,
    medium_purple1     = 141,
    medium_purple2     = 135,
    medium_purple2_1   = 140,
    medium_purple3     = 97,
    medium_purple3_1   = 98,
    medium_purple4     = 60,
    medium_spring_green = 49,
    medium_turquoise   = 80,
    medium_violet_red   = 126,
    misty_rose1        = 224,
    misty_rose3        = 181,
    navajo_white1      = 223,
    navajo_white3      = 144,
    navy_blue          = 17,
    orange1           = 214,
    orange3           = 172,
    orange4           = 58,
    orange4_1         = 94,
    orange_red1        = 202,
    orchid            = 170,
    orchid1           = 213,
    orchid2           = 212,
    pale_green1        = 121,
    pale_green1_1      = 156,
    pale_green3        = 77,
    pale_green3_1      = 114,
    pale_turquoise1    = 159,
    pale_turquoise4    = 66,
    pale_violet_red1    = 211,
    pink1             = 218,
    pink3             = 175,
    plum1             = 219,
    plum2             = 183,
    plum3             = 176,
    plum4             = 96,
    purple            = 93,
    purple3           = 56,
    purple4           = 54,
    purple4_1         = 55,
    purple_1          = 129,
    red1              = 196,
    red3              = 124,
    red3_1            = 160,
    rosy_brown         = 138,
    royal_blue1        = 63,
    salmon1           = 209,
    sandy_brown        = 215,
    sea_green1         = 84,
    sea_green1_1       = 85,
    sea_green2         = 83,
    sea_green3         = 78,
    sky_blue1          = 117,
    sky_blue2          = 111,
    sky_blue3          = 74,
    slate_blue1        = 99,
    slate_blue3        = 61,
    slate_blue3_1      = 62,
    spring_green1      = 48,
    spring_green2      = 42,
    spring_green2_1    = 47,
    spring_green3      = 35,
    spring_green3_1    = 41,
    spring_green4      = 29,
    steel_blue         = 67,
    steel_blue1        = 75,
    steel_blue1_1      = 81,
    steel_blue3        = 68,
    tan               = 180,
    thistle1          = 225,
    thistle3          = 182,
    turquoise2        = 45,
    turquoise4        = 30,
    violet            = 177,
    wheat1            = 229,
    wheat4            = 101,
    yellow1           = 226,
    yellow2           = 190,
    yellow3           = 148,
    yellow3_1         = 184,
    yellow4           = 100,
    yellow4_1         = 106,
};

// clang-format on
// NOLINTEND

} // namespace deco::colors

namespace deco::detail {

// ──────────────────────── Terminal color info ─────────────────────

struct RGB {
    uint8_t r, g, b;
};

// NOLINTBEGIN
inline constexpr std::array<RGB, 256> color_info {
    {{0, 0, 0},       {128, 0, 0},     {0, 128, 0},     {128, 128, 0},
     {0, 0, 128},     {128, 0, 128},   {0, 128, 128},   {192, 192, 192},
     {128, 128, 128}, {255, 0, 0},     {0, 255, 0},     {255, 255, 0},
     {0, 0, 255},     {255, 0, 255},   {0, 255, 255},   {255, 255, 255},
     {0, 0, 0},       {0, 0, 95},      {0, 0, 135},     {0, 0, 175},
     {0, 0, 215},     {0, 0, 255},     {0, 95, 0},      {0, 95, 95},
     {0, 95, 135},    {0, 95, 175},    {0, 95, 215},    {0, 95, 255},
     {0, 135, 0},     {0, 135, 95},    {0, 135, 135},   {0, 135, 175},
     {0, 135, 215},   {0, 135, 255},   {0, 175, 0},     {0, 175, 95},
     {0, 175, 135},   {0, 175, 175},   {0, 175, 215},   {0, 175, 255},
     {0, 215, 0},     {0, 215, 95},    {0, 215, 135},   {0, 215, 175},
     {0, 215, 215},   {0, 215, 255},   {0, 255, 0},     {0, 255, 95},
     {0, 255, 135},   {0, 255, 175},   {0, 255, 215},   {0, 255, 255},
     {95, 0, 0},      {95, 0, 95},     {95, 0, 135},    {95, 0, 175},
     {95, 0, 215},    {95, 0, 255},    {95, 95, 0},     {95, 95, 95},
     {95, 95, 135},   {95, 95, 175},   {95, 95, 215},   {95, 95, 255},
     {95, 135, 0},    {95, 135, 95},   {95, 135, 135},  {95, 135, 175},
     {95, 135, 215},  {95, 135, 255},  {95, 175, 0},    {95, 175, 95},
     {95, 175, 135},  {95, 175, 175},  {95, 175, 215},  {95, 175, 255},
     {95, 215, 0},    {95, 215, 95},   {95, 215, 135},  {95, 215, 175},
     {95, 215, 215},  {95, 215, 255},  {95, 255, 0},    {95, 255, 95},
     {95, 255, 135},  {95, 255, 175},  {95, 255, 215},  {95, 255, 255},
     {135, 0, 0},     {135, 0, 95},    {135, 0, 135},   {135, 0, 175},
     {135, 0, 215},   {135, 0, 255},   {135, 95, 0},    {135, 95, 95},
     {135, 95, 135},  {135, 95, 175},  {135, 95, 215},  {135, 95, 255},
     {135, 135, 0},   {135, 135, 95},  {135, 135, 135}, {135, 135, 175},
     {135, 135, 215}, {135, 135, 255}, {135, 175, 0},   {135, 175, 95},
     {135, 175, 135}, {135, 175, 175}, {135, 175, 215}, {135, 175, 255},
     {135, 215, 0},   {135, 215, 95},  {135, 215, 135}, {135, 215, 175},
     {135, 215, 215}, {135, 215, 255}, {135, 255, 0},   {135, 255, 95},
     {135, 255, 135}, {135, 255, 175}, {135, 255, 215}, {135, 255, 255},
     {175, 0, 0},     {175, 0, 95},    {175, 0, 135},   {175, 0, 175},
     {175, 0, 215},   {175, 0, 255},   {175, 95, 0},    {175, 95, 95},
     {175, 95, 135},  {175, 95, 175},  {175, 95, 215},  {175, 95, 255},
     {175, 135, 0},   {175, 135, 95},  {175, 135, 135}, {175, 135, 175},
     {175, 135, 215}, {175, 135, 255}, {175, 175, 0},   {175, 175, 95},
     {175, 175, 135}, {175, 175, 175}, {175, 175, 215}, {175, 175, 255},
     {175, 215, 0},   {175, 215, 95},  {175, 215, 135}, {175, 215, 175},
     {175, 215, 215}, {175, 215, 255}, {175, 255, 0},   {175, 255, 95},
     {175, 255, 135}, {175, 255, 175}, {175, 255, 215}, {175, 255, 255},
     {215, 0, 0},     {215, 0, 95},    {215, 0, 135},   {215, 0, 175},
     {215, 0, 215},   {215, 0, 255},   {215, 95, 0},    {215, 95, 95},
     {215, 95, 135},  {215, 95, 175},  {215, 95, 215},  {215, 95, 255},
     {215, 135, 0},   {215, 135, 95},  {215, 135, 135}, {215, 135, 175},
     {215, 135, 215}, {215, 135, 255}, {215, 175, 0},   {215, 175, 95},
     {215, 175, 135}, {215, 175, 175}, {215, 175, 215}, {215, 175, 255},
     {215, 215, 0},   {215, 215, 95},  {215, 215, 135}, {215, 215, 175},
     {215, 215, 215}, {215, 215, 255}, {215, 255, 0},   {215, 255, 95},
     {215, 255, 135}, {215, 255, 175}, {215, 255, 215}, {215, 255, 255},
     {255, 0, 0},     {255, 0, 95},    {255, 0, 135},   {255, 0, 175},
     {255, 0, 215},   {255, 0, 255},   {255, 95, 0},    {255, 95, 95},
     {255, 95, 135},  {255, 95, 175},  {255, 95, 215},  {255, 95, 255},
     {255, 135, 0},   {255, 135, 95},  {255, 135, 135}, {255, 135, 175},
     {255, 135, 215}, {255, 135, 255}, {255, 175, 0},   {255, 175, 95},
     {255, 175, 135}, {255, 175, 175}, {255, 175, 215}, {255, 175, 255},
     {255, 215, 0},   {255, 215, 95},  {255, 215, 135}, {255, 215, 175},
     {255, 215, 215}, {255, 215, 255}, {255, 255, 0},   {255, 255, 95},
     {255, 255, 135}, {255, 255, 175}, {255, 255, 215}, {255, 255, 255},
     {8, 8, 8},       {18, 18, 18},    {28, 28, 28},    {38, 38, 38},
     {48, 48, 48},    {58, 58, 58},    {68, 68, 68},    {78, 78, 78},
     {88, 88, 88},    {98, 98, 98},    {108, 108, 108}, {118, 118, 118},
     {128, 128, 128}, {138, 138, 138}, {148, 148, 148}, {158, 158, 158},
     {168, 168, 168}, {178, 178, 178}, {188, 188, 188}, {198, 198, 198},
     {208, 208, 208}, {218, 218, 218}, {228, 228, 228}, {238, 238, 238}}};
// NOLINTEND

} // namespace deco::detail

#endif // !DECOTERM_COLOR_INFO_HPP


#ifndef DECOTERM_OUTPUT_HPP
#define DECOTERM_OUTPUT_HPP


#include <array>
#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>

namespace deco {

/// Terminal color capability levels.
enum class ColorSupport : uint8_t { Color16, Color256, TrueColor };

namespace detail {

template <typename Ptr>
    requires std::is_pointer_v<Ptr>
struct NotNull {
    NotNull(Ptr ptr) : ptr_(ptr) {
        assert(ptr != nullptr && "deco::detail::NotNull(): null pointer");
    }

    auto operator->() const -> Ptr { return ptr_; }
    auto operator*() const -> decltype(auto) { return *ptr_; }

    operator Ptr() const { return ptr_; }

    auto get() const -> Ptr { return ptr_; }

  private:
    Ptr ptr_;
};

template <typename T, std::size_t N>
    requires(N > 0 && std::is_copy_assignable_v<T>)
class Buffer {
  public:
    using value_type = T;

    constexpr Buffer() = default; // NOLINT

    constexpr Buffer(const Buffer& other) { this->copy_from(other); }
    constexpr Buffer(Buffer&& other) noexcept {
        this->move_from(std::move(other));
    }

    constexpr auto operator=(const Buffer& other) -> Buffer& {
        if (this == &other) return *this;
        this->copy_from(other);
        return *this;
    }

    constexpr auto operator=(Buffer&& other) noexcept -> Buffer& {
        if (this == &other) return *this;
        this->move_from(std::move(other));
        return *this;
    }

    constexpr ~Buffer() = default;

    [[nodiscard]] constexpr auto data() -> T* { return data_; }

    [[nodiscard]] constexpr auto back() -> T& {
        assert(!this->empty());
        return data_[size_ - 1];
    }

    constexpr void clear() { size_ = 0; }

    constexpr void push_back(T v) {
        if (size_ == capacity_) this->grow();
        data_[size_++] = v; // NOLINT
    }

    constexpr void pop_back() {
        if (size_ > 0) size_--;
    }

    [[nodiscard]] constexpr auto data() const -> const T* { return data_; }
    [[nodiscard]] constexpr auto back() const -> T {
        assert(!this->empty());
        return data_[size_ - 1];
    }
    [[nodiscard]] constexpr auto back_or(T v) const -> T {
        return this->empty() ? v : this->back();
    }
    [[nodiscard]] constexpr auto size() const -> std::size_t { return size_; }
    [[nodiscard]] constexpr auto empty() const -> bool { return !size_; }

  private:
    constexpr auto is_heap() const noexcept -> bool {
        return data_ == heap_.get();
    }

    constexpr void copy_from(const Buffer& other) {
        if (other.is_heap()) {
            heap_ = std::make_unique_for_overwrite<T[]>(other.capacity_);
            for (std::size_t i = 0; i < other.size_; ++i)
                heap_[i] = other.heap_[i];
            data_ = heap_.get();
        } else {
            store_ = other.store_;
            data_ = store_.data();
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    constexpr void move_from(Buffer&& other) noexcept {
        if (other.is_heap()) {
            heap_ = std::move(other.heap_);
            data_ = heap_.get();
        } else {
            store_ = other.store_;
            data_ = store_.data();
        }

        size_ = other.size_;
        capacity_ = other.capacity_;

        other.size_ = 0;
        other.capacity_ = N;
        other.data_ = other.store_.data();
    }

    constexpr void grow() {
        const std::size_t new_capacity =
            capacity_ / 2 == 0 ? capacity_ * 2 : capacity_ + (capacity_ / 2);
        auto new_heap = std::make_unique_for_overwrite<T[]>(new_capacity);
        for (std::size_t i = 0; i < size_; ++i)
            new_heap[i] = data_[i];
        heap_ = std::move(new_heap);
        data_ = heap_.get();
        capacity_ = new_capacity;
    }

    std::array<T, N> store_;
    std::unique_ptr<T[]> heap_;

    T* data_ = store_.data();
    std::size_t size_ = 0;
    std::size_t capacity_ = N;
};

/* ----- color fallback ----- */

// TODO: replace this with better algorithm
constexpr auto rgb_distance(detail::RGB lhs, detail::RGB rhs) -> int {
    const int dr = lhs.r - rhs.r;
    const int dg = lhs.g - rhs.g;
    const int db = lhs.b - rhs.b;
    return (2 * (dr * dr)) + (4 * (db * db)) + (3 * (dg * dg));
}

constexpr auto fallback_to_16(Color color) -> Color {
    const ColorType type = color.type();
    if (type == ColorType::Null || type == ColorType::DefaultColor) {
        return color;
    }

    RGB rgb = {.r = 0, .g = 0, .b = 0};
    if (type == ColorType::TerminalColor) {
        const uint8_t index = color.data()[0];
        if (index < 16) return color;
        rgb = color_info[index];
    } else if (type == ColorType::TrueColor) {
        const auto [r, g, b] = color.data();
        rgb = RGB(r, g, b);
    }

    int closest = (256 * 256) * 3; // max distance
    uint8_t closest_idx = 0;
    for (uint8_t i = 0; i < 16; ++i) {
        int distance = rgb_distance(rgb, color_info[i]);
        if (distance < closest) {
            closest = distance;
            closest_idx = i;
        }
    }

    return {closest_idx};
}

constexpr auto fallback_to_256(Color color) -> Color {
    // TODO:
    return color;
}

constexpr auto fallback_color(Style style, ColorSupport mode) -> Style {
    switch (mode) {
        case ColorSupport::Color16:
            return Style(style.emphasis(),
                         fallback_to_16(style.fg()),
                         fallback_to_16(style.bg()));
        case ColorSupport::Color256:
            return Style(style.emphasis(),
                         fallback_to_256(style.fg()),
                         fallback_to_256(style.bg()));
        default:
            return style;
    }
}

constexpr auto fallback_color(AbsoluteStyle abstyle, ColorSupport mode)
    -> AbsoluteStyle {
    return abs(fallback_color(abstyle.inner(), mode));
}

constexpr auto disable_color(Style style) -> Style {
    return Style(style.emphasis());
}

constexpr auto disable_color(AbsoluteStyle style) -> AbsoluteStyle {
    return abs(Style(style.inner().emphasis()));
}

/* ----- style merge ----- */

constexpr auto merge(Style lhs, Style rhs) -> Style { return lhs | rhs; }

constexpr auto merge(AbsoluteStyle lhs, Style rhs) -> AbsoluteStyle {
    return abs(lhs.inner() | rhs);
}

constexpr auto merge([[maybe_unused]] Style lhs, AbsoluteStyle rhs)
    -> AbsoluteStyle {
    return rhs;
}

constexpr auto merge([[maybe_unused]] AbsoluteStyle lhs, AbsoluteStyle rhs)
    -> AbsoluteStyle {
    return rhs;
}

struct AnyStyle {
    constexpr AnyStyle() : type_(Type::Style) {}
    constexpr AnyStyle(Style style) : type_(Type::Style), inner_(style) {}
    constexpr AnyStyle(AbsoluteStyle abstyle)
        : type_(Type::AbsoluteStyle),
          inner_(abstyle.inner()) {}

    constexpr void merge(detail::style_type auto s) {
        switch (type_) {
            case Type::Style:
                *this = detail::merge(inner_, s);
                break;
            case Type::AbsoluteStyle:
                *this = detail::merge(abs(inner_), s);
                break;
        }
    }

    template <typename Visitor>
    constexpr auto visit(Visitor&& visitor) const -> decltype(auto) { // NOLINT
        switch (type_) {
            case Type::Style:
                return visitor(inner_);
            case Type::AbsoluteStyle:
                return visitor(abs(inner_));
        }
        assert(false);
        return visitor(inner_); // unreachable
    }

    constexpr auto is_null() const -> bool {
        return this->visit(
            [](detail::style_type auto style) { return style.is_null(); });
    }

    constexpr auto inner() const -> Style { return inner_; }

  private:
    enum class Type { Style, AbsoluteStyle };

    Type type_;
    Style inner_;
};

template <std::size_t N>
using StyleStack = Buffer<AbsoluteStyle, N>;

class StyleContext {
  public:
    constexpr StyleContext() = default;

    constexpr void set_base_style(Style s) {
        AbsoluteStyle base = abs(s);
        if (stack_.empty()) pending_.merge(base);
        base_style_ = base;
    }

    /// @details
    /// This will push `style` to the stack implicitly when the stack is empty.
    constexpr void merge_current_style(detail::style_type auto style) {
        this->merge_current_style_no_pending(style);
        pending_.merge(style);
    }

    constexpr void merge_current_style_no_pending(
        detail::style_type auto style) {
        if (stack_.empty()) stack_.push_back(detail::merge(base_style_, style));
        else stack_.back() = detail::merge(stack_.back(), style);
    }

    constexpr void ensure_style() { pending_.merge(this->current_abstyle()); }

    constexpr void push(detail::style_type auto style) {
        stack_.push_back(detail::merge(stack_.back_or(base_style_), style));
        pending_.merge(style);
    }

    constexpr void pop() {
        stack_.pop_back();
        pending_.merge(this->current_abstyle());
    }

    constexpr void reset() {
        stack_.clear();
        pending_.merge(this->current_abstyle());
    }

    constexpr void drop_pending() { pending_ = null_style; }

    [[nodiscard]] constexpr auto consume_pending() -> AnyStyle {
        auto pending = pending_;
        pending_ = null_style;
        return pending;
    }

    [[nodiscard]] constexpr auto base_style() const -> Style {
        return base_style_.inner();
    }

    [[nodiscard]] constexpr auto pending() const -> AnyStyle {
        return pending_;
    }

    [[nodiscard]] constexpr auto has_pending() const -> bool {
        return !pending_.is_null();
    }

    [[nodiscard]] constexpr auto current_abstyle() const -> AbsoluteStyle {
        return stack_.back_or(base_style_);
    }

  private:
    AbsoluteStyle base_style_;

    StyleStack<3> stack_;
    AnyStyle pending_;
};

template <detail::style_type StyleT>
struct Push {
    StyleT style;
};

struct Pop {};

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                      OutputConfig                       ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Configures how styles and colors are emitted.
///
/// @details
/// - This class is **thread-safe**. If an object holds a reference to an
///   `OutputConfig`, modifying the configuration concurrently does not cause a
///   data race.
class OutputConfig {
  public:
    OutputConfig() = default;

    /// @brief Enables or disables color output. Default: `true`
    /// @details When disabled, styles still emit their emphasis attributes.
    auto enable_color(bool enable = true) -> OutputConfig& {
        color_enabled_.store(enable, std::memory_order_relaxed);
        return *this;
    }

    /// Enables or disables all style output. Default: `true`
    auto enable_style(bool enable = true) -> OutputConfig& {
        style_enabled_.store(enable, std::memory_order_relaxed);
        return *this;
    }

    /// Sets the color capability of the output terminal. Default: `TrueColor`
    auto set_color_support(ColorSupport mode) -> OutputConfig& {
        color_support_.store(mode, std::memory_order_relaxed);
        return *this;
    }

    [[nodiscard]] auto color_enabled() const -> bool {
        return color_enabled_.load(std::memory_order_relaxed);
    }

    [[nodiscard]] auto style_enabled() const -> bool {
        return style_enabled_.load(std::memory_order_relaxed);
    }

    [[nodiscard]] auto color_support() const -> ColorSupport {
        return color_support_.load(std::memory_order_relaxed);
    }

  private:
    std::atomic_bool color_enabled_ = true;
    std::atomic_bool style_enabled_ = true;
    std::atomic<ColorSupport> color_support_ = ColorSupport::TrueColor;
};

/// Returns the global output config.
inline auto config() -> OutputConfig& {
    static OutputConfig global_cfg;
    return global_cfg;
}

// ╔═════════════════════════════════════════════════════════╗
// ║                         OStream                         ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A wrapper around `std::ostream` that tracks style context and can be
/// configured through `OutputConfig`.
///
/// @details
/// Unlike writing styles directly to a `std::ostream`, an `OStream` keeps track
/// of the current style and related context internally. This results in several
/// behavioral differences:
///
/// - After writing a `styled` value, the previous style is restored instead of
///   resetting the style entirely.
///
/// - Writing `reset` restores the `base_style` and also clears the entire style
///   context.
///
/// - Styles are not emitted immediately. Instead, pending style changes are
///   emitted together with subsequent output when needed. This allows
///   consecutive style changes to be combined and reduces the amount of
///   generated output.
///
/// - The style stack can be controlled with the `push(style)` and `pop`
///   manipulators.
///
/// **Example**
///
/// ```cpp
/// OStream os(std::cout);
///
/// // fg(blue) and bold is merged to fg(blue) | bold
/// os << fg(blue) << bold << "text";
///
/// // same as `os << italic << "Hello" << abs(fg(blue) | bold) << "World"`
/// os << styled("Hello", italic) << "World";
///
/// // nested style
/// // same as `os << italic << "Hello" << abs(fg(blue) | bold) << "World`
/// os << push(italic) << "Hello" << pop << "World";
/// ```
///
/// An `OStream` may also reference an `OutputConfig` object to control how
/// styles and colors are emitted. This includes enabling or disabling style
/// and color output, as well as color fallback behavior.
///
/// @warning
/// - The internal style context is valid only as long as styles are not written
///   to the underlying stream by other means during the lifetime of the
///   `OStream`. Call `ensure_style()` to restore consistency if this happens.
///
/// - This class is intended for single-threaded use. Concurrent access to the
///   same `OStream` object may cause data races.
///
/// - The `OStream` object must not outlive the underlying `std::ostream` or the
///   referenced `OutputConfig`, if any.
class OStream {
  public:
    explicit OStream(std::ostream& ostream)
        : cfg_(nullptr),
          ostream_(&ostream) {
        ctx_.ensure_style();
    }

    explicit OStream(const OutputConfig& config, std::ostream& ostream)
        : cfg_(&config),
          ostream_(&ostream) {
        ctx_.ensure_style();
    }

    auto set_base_style(Style s) -> OStream& {
        ctx_.set_base_style(s);
        return *this;
    }

    /// Emits the current style.
    auto ensure_style() -> OStream& {
        ctx_.ensure_style();
        return *this;
    }

    [[nodiscard]] auto current_style() const -> Style {
        return ctx_.current_abstyle().inner();
    }

    [[nodiscard]] auto base_style() const -> Style { return ctx_.base_style(); }

    /// Returns the underlying `std::ostream`.
    [[nodiscard]] auto ostream() const -> std::ostream& { return *ostream_; }

    /// Clones this `OStream` with different underlying `std::ostream`.
    [[nodiscard]] auto clone_with(std::ostream& new_ostream) const -> OStream {
        return OStream(cfg_, new_ostream, ctx_);
    }

    /* ----- output operators ----- */

    /// @brief Output operator for any type that can be written to
    /// `std::ostream`.
    ///
    /// @throws `std::logic_error` if `operator<<(std::ostream&, T&&)` returns a
    /// different ostream object.
    template <ostream_outputable T>
        requires(!detail::style_type<T> && !detail::styled<T>)
    friend auto operator<<(OStream& out, T&& value) -> OStream& {
        out.output_pending();
        auto os_ptr = &(out.ostream() << std::forward<T>(value));
        if (os_ptr != out.ostream_)
            throw std::logic_error(
                "deco::OStream: std::ostream output operator returned a "
                "different ostream object");
        return out;
    }

    friend auto operator<<(OStream& out, Style style) -> OStream& {
        out.ctx_.merge_current_style(style);
        return out;
    }

    friend auto operator<<(OStream& out, AbsoluteStyle abstyle) -> OStream& {
        out.ctx_.merge_current_style(abstyle);
        return out;
    }

    template <detail::style_type StyleT, ostream_outputable T>
    friend auto operator<<(OStream& out,
                           const detail::Styled<StyleT, T>& styled)
        -> OStream& {
        auto& ctx = out.ctx_;
        if (ctx.has_pending()) {
            auto pending = ctx.consume_pending();
            pending.merge(styled.style());
            pending.visit([&out](auto style) { out.output_style(style); });
        } else {
            out.output_style(styled.style());
        }
        out << styled.value();
        if (!styled.style().is_null()) out.output_style(ctx.current_abstyle());
        return out;
    }

    template <detail::style_type StyleT>
    friend auto operator<<(OStream& out, detail::Push<StyleT> push)
        -> OStream& {
        out.ctx_.push(push.style);
        return out;
    }

    friend auto operator<<(OStream& out, detail::Pop) -> OStream& {
        out.ctx_.pop();
        return out;
    }

    friend auto operator<<(OStream& out, Reset) -> OStream& {
        out.ctx_.reset();
        return out;
    }

    /// output operator for IO manipulators
    friend auto operator<<(OStream& out, std::ios_base& (*fn)(std::ios_base&))
        -> OStream& {
        out.output_pending();
        out.ostream() << fn;
        return out;
    }

    /// output operator for IO manipulators
    friend auto operator<<(OStream& out,
                           std::basic_ios<char>& (*fn)(std::basic_ios<char>&))
        -> OStream& {
        out.output_pending();
        out.ostream() << fn;
        return out;
    }

    /// output operator for IO manipulators
    friend auto operator<<(OStream& out, std::ostream& (*fn)(std::ostream&))
        -> OStream& {
        // `std::operator<<(std::ostream& os,
        // std::ostream&(*fn)(std::ostream&))` returns `fn(os)` instead of `os`,
        // so we should assume that it may return a different std::ostream&.
        out.output_pending();
        out.ostream_ = &(out.ostream() << fn);
        return out;
    }

  private:
    explicit OStream(const OutputConfig* config,
                     std::ostream& ostream,
                     detail::StyleContext ctx)
        : ctx_(std::move(ctx)),
          cfg_(config),
          ostream_(&ostream) {
        ctx_.drop_pending();
        ctx_.ensure_style();
    }

    void output_pending() {
        if (ctx_.has_pending()) {
            const auto pending = ctx_.consume_pending();
            pending.visit([this](detail::style_type auto style) constexpr {
                this->output_style(style);
            });
        }
    }

    template <detail::style_type StyleT>
    void output_style(StyleT style) {
        if (!cfg_) {
            *ostream_ << style;
            return;
        }
        if (!cfg_->style_enabled()) return;
        if (!cfg_->color_enabled()) style = detail::disable_color(style);
        else style = detail::fallback_color(style, cfg_->color_support());
        *ostream_ << style;
    }

    detail::StyleContext ctx_;

    const OutputConfig* cfg_;
    detail::NotNull<std::ostream*> ostream_;
};

/// Creates a `OStream` that links to `std::cout` and global output config.
inline auto stdout_ostream() -> OStream { return OStream(config(), std::cout); }

/// Creates a `OStream` that links to `std::cerr` and global output config.
inline auto stderr_ostream() -> OStream { return OStream(config(), std::cerr); }

/* ----- OStream manipulators ----- */

[[nodiscard]] constexpr auto push(Style style) -> detail::Push<Style> {
    return {style};
}
[[nodiscard]] constexpr auto push(AbsoluteStyle abstyle)
    -> detail::Push<AbsoluteStyle> {
    return {abstyle};
}

inline constexpr detail::Pop pop {};

} // namespace deco

#endif // !DECOTERM_OUTPUT_HPP


#ifndef DECOTERM_TERMINAL_HPP
#define DECOTERM_TERMINAL_HPP


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

/// Detects the current terminal environment and returns the result as a `TerminalInfo`.
[[nodiscard]]
inline auto terminal_info() -> TerminalInfo {
    return {
        .color_support = detail::get_color_support(),
        .is_stdout_tty = detail::is_stdout_tty(),
        .is_stderr_tty = detail::is_stderr_tty(),
        .no_color = detail::no_color(),
    };
}

/// @brief Configures the global output config based on the current terminal
/// environment.
///
/// @details
/// - If the `NO_COLOR` environment variable is set, color output is disabled.
///   (Style output remains enabled)
/// -  `color_support` are set according to the detected `ColorSupport`.
///
/// On Windows, this function also attempts to enable Virtual Terminal (VT)
/// processing for `stdout` and `stderr` if they are attached to a terminal.
///
/// @return The detected `TerminalInfo`.
///
/// @throws std::runtime_error If enabling VT processing fails on Windows.
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


#ifndef DECOTERM_FORMAT_HPP
#define DECOTERM_FORMAT_HPP


#include <format>
#include <iterator>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <variant>
#include <version>

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L
#define DECO_ENABLE_STD_PRINT
#include <cstdio>
#include <print>
#endif

namespace deco {

#if __cplusplus >= 202302L
template <typename Arg, typename CharT = char>
concept formattable = std::formattable<Arg, CharT>;
#else
// fallback for C++20. This only checks if `std::formatter<Arg>` exists
// and has `parse()` and `format()`.
template <typename Arg, typename CharT = char>
concept formattable =
    requires(std::formatter<std::remove_cvref_t<Arg>, CharT> formatter,
             Arg&& arg,
             std::basic_format_parse_context<CharT> parse_ctx,
             std::basic_format_context<char*, CharT> fmt_ctx) {
        {
            formatter.parse(parse_ctx)
        } -> std::same_as<typename decltype(parse_ctx)::iterator>;
        {
            formatter.format(std::forward<Arg>(arg), fmt_ctx)
        } -> std::same_as<typename decltype(fmt_ctx)::iterator>;
    };
#endif

namespace detail {

template <typename T>
struct formatter_type {
    using type = std::formatter<T>;
};

template <typename Char, std::size_t N>
    requires requires { std::formatter<std::basic_string_view<Char>, Char>(); }
struct formatter_type<Char[N]> {
    using type = std::formatter<std::basic_string_view<Char>, Char>;
};

template <typename T>
using formatter_t = formatter_type<std::remove_cvref_t<T>>::type;

// inspired by {fmt} library
inline constexpr std::size_t FMT_INLINE_BUFFER_SIZE = 500;

using FormatBuf = Buffer<char, FMT_INLINE_BUFFER_SIZE>;

template <typename T, std::size_t N>
struct BufAppender {
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    explicit constexpr BufAppender(Buffer<T, N>& buf) : buf(&buf) {}

    constexpr auto operator*() -> BufAppender& { return *this; }
    constexpr auto operator++() -> BufAppender& { return *this; }
    constexpr auto operator++(int) -> BufAppender& { return *this; }

    constexpr auto operator=(T v) -> BufAppender& {
        buf->push_back(v);
        return *this;
    }

    NotNull<Buffer<T, N>*> buf;
};

template <style_type StyleT, typename T>
struct FStyled {
    constexpr FStyled(Styled<StyleT, T> styled,
                      AbsoluteStyle current_style,
                      const OutputConfig* cfg)
        : styled(std::move(styled)),
          restore_style(current_style),
          cfg(cfg) {}

    Styled<StyleT, T> styled;
    AbsoluteStyle restore_style;
    const OutputConfig* cfg;
};

// Replace `Styled` with `FStyled`
template <typename Arg>
constexpr auto process_fmt_arg(Arg& arg,
                               AbsoluteStyle current_style,
                               const OutputConfig* cfg) -> decltype(auto) {
    static_assert(
        !style_type<Arg> && !std::is_same_v<std::remove_cvref_t<Arg>, Reset>,
        "Style types and 'reset' cannot be used as a format argument.");
    if constexpr (detail::styled<Arg>)
        return FStyled(std::move(arg), current_style, cfg);
    else return arg;
}

template <typename... Args>
inline auto make_format_args(Args&&... args /* NOLINT */) {
    return std::make_format_args(args...);
}

template <std::output_iterator<const char&> OutputIt,
          style_type StyleT,
          typename... Args>
inline auto format_to_impl(OutputIt out,
                           StyleT style,
                           std::format_string<Args...> fmt,
                           Args&&... args /* NOLINT */) -> OutputIt {
    out = style.to_escape(out);
    out = std::vformat_to(
        out,
        fmt.get(),
        make_format_args(process_fmt_arg(args, abs(style), nullptr)...));
    if (!style.is_null()) out = abs(null_style).to_escape(out);
    return out;
}

template <style_type StyleT, typename... Args>
inline auto format_impl(StyleT style,
                        std::format_string<Args...> fmt,
                        Args&&... args /* NOLINT */) -> std::string {
    auto buf = FormatBuf();
    format_to_impl(BufAppender(buf), style, fmt, std::forward<Args>(args)...);
    return {buf.data(), buf.size()};
}

template <std::output_iterator<const char&> OutputIt, detail::style_type StyleT>
auto write_style(OutputIt out, StyleT style, const OutputConfig* cfg)
    -> OutputIt {
    if (!cfg) return style.to_escape(out);
    if (!cfg->style_enabled()) return out;
    if (!cfg->color_enabled()) style = detail::disable_color(style);
    else style = detail::fallback_color(style, cfg->color_support());
    return style.to_escape(out);
}

#ifdef DECO_ENABLE_STD_PRINT

template <typename Stream, style_type StyleT, typename... Args>
inline auto print_impl(Stream&& stream /* NOLINT */,
                       StyleT style,
                       std::format_string<Args...> fmt,
                       Args&&... args) {
    auto buffer = FormatBuf();
    format_to_impl(
        BufAppender(buffer), style, fmt, std::forward<Args>(args)...);
    std::print(stream, "{}", std::string_view(buffer.data(), buffer.size()));
}

template <typename Stream, style_type StyleT, typename... Args>
inline auto println_impl(Stream&& stream /* NOLINT */,
                         StyleT style,
                         std::format_string<Args...> fmt,
                         Args&&... args) {
    auto buffer = FormatBuf();
    format_to_impl(
        BufAppender(buffer), style, fmt, std::forward<Args>(args)...);
    std::println(stream, "{}", std::string_view(buffer.data(), buffer.size()));
}

#endif // DECO_ENABLE_STD_PRINT

} // namespace detail

} // namespace deco

// ╔═════════════════════════════════════════════════════════╗
// ║                       formatters                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

/// formatter for style types
template <deco::detail::style_type StyleT>
struct formatter<StyleT> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(StyleT style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

/// formatter for `deco::reset`
template <>
struct formatter<deco::Reset> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::Reset, std::format_context& ctx) const {
        return deco::abs(deco::null_style).to_escape(ctx.out());
    }
};

/// formatter for `styled()`
template <deco::detail::style_type StyleT, typename T>
    requires deco::formattable<T>
struct formatter<deco::detail::Styled<StyleT, T>>
    : deco::detail::formatter_t<T> {
    auto format(const deco::detail::Styled<StyleT, T>& styled,
                std::format_context& ctx) const {
        using namespace deco;

        ctx.advance_to(styled.style().to_escape(ctx.out()));
        ctx.advance_to(detail::formatter_t<T>::format(styled.value(), ctx));
        ctx.advance_to(abs(null_style).to_escape(ctx.out()));

        return ctx.out();
    }
};

template <deco::detail::style_type StyleT, typename T>
    requires deco::formattable<T>
struct formatter<deco::detail::FStyled<StyleT, T>>
    : deco::detail::formatter_t<T> {
    auto format(const deco::detail::FStyled<StyleT, T>& fstyled,
                std::format_context& ctx) const {
        using namespace deco;

        const auto& styled = fstyled.styled;

        ctx.advance_to(
            detail::write_style(ctx.out(), styled.style(), fstyled.cfg));
        ctx.advance_to(detail::formatter_t<T>::format(styled.value(), ctx));
        ctx.advance_to(
            detail::write_style(ctx.out(), fstyled.restore_style, fstyled.cfg));

        return ctx.out();
    }
};

} // namespace std

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                         format                          ║
// ╚═════════════════════════════════════════════════════════╝

template <std::output_iterator<const char&> OutputIt, typename... Args>
inline auto format_to(OutputIt out,
                      Style style,
                      std::format_string<Args...> fmt,
                      Args&&... args) -> OutputIt {
    return detail::format_to_impl(out, style, fmt, std::forward<Args>(args)...);
}

template <std::output_iterator<const char&> OutputIt, typename... Args>
inline auto format_to(OutputIt out,
                      AbsoluteStyle abstyle,
                      std::format_string<Args...> fmt,
                      Args&&... args) -> OutputIt {
    return detail::format_to_impl(
        out, abstyle, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] inline auto format(Style style,
                                 std::format_string<Args...> fmt,
                                 Args&&... args) -> std::string {
    return detail::format_impl(style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] inline auto format(AbsoluteStyle abstyle,
                                 std::format_string<Args...> fmt,
                                 Args&&... args) -> std::string {
    return detail::format_impl(abstyle, fmt, std::forward<Args>(args)...);
}

#ifdef DECO_ENABLE_STD_PRINT

// ╔═════════════════════════════════════════════════════════╗
// ║                          print                          ║
// ╚═════════════════════════════════════════════════════════╝

template <typename... Args>
inline void print(Style s, std::format_string<Args...> fmt, Args&&... args) {
    detail::print_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(FILE* f,
                  Style s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(FILE* f,
                  AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::ostream& os,
                  Style s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::ostream& os,
                  AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(Style s, std::format_string<Args...> fmt, Args&&... args) {
    detail::println_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(FILE* f,
                    Style s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(FILE* f,
                    AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::ostream& os,
                    Style s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::ostream& os,
                    AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl(os, s, fmt, std::forward<Args>(args)...);
}

// ╔═════════════════════════════════════════════════════════╗
// ║                         Printer                         ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Provides functionality similar to `deco::OStream`, but uses a
/// `std::print`-style API instead of stream insertion.
///
/// @details
/// - Use the `push()` and `pop()` member functions to manage the style stack,
///   rather than manipulators.
/// - `style` and `reset` cannot be used as format arguments because `Printer`
///   cannot track them and update its internal style context accordingly.
///   Use `apply()` to modify the current style and `reset()` to reset the style
///   context instead.
class Printer {
  public:
    explicit Printer(std::ostream& os) : cfg_(nullptr), stream(&os) {
        ctx_.ensure_style();
    }

    explicit Printer(std::FILE* f = stdout) : cfg_(nullptr), stream(f) {
        ctx_.ensure_style();
    }

    explicit Printer(const OutputConfig& config, std::ostream& os)
        : cfg_(&config),
          stream(&os) {
        ctx_.ensure_style();
    }

    explicit Printer(const OutputConfig& config, std::FILE* f = stdout)
        : cfg_(&config),
          stream(f) {
        ctx_.ensure_style();
    }

    auto set_base_style(Style s) -> Printer& {
        ctx_.set_base_style(s);
        return *this;
    }

    /// Emits the current style.
    auto ensure_style() -> Printer& {
        ctx_.ensure_style();
        return *this;
    }

    /// Changes the current style.
    auto apply(Style s) -> Printer& {
        ctx_.merge_current_style(s);
        return *this;
    }

    auto push(Style s) -> Printer& {
        ctx_.push(s);
        return *this;
    }

    auto push(AbsoluteStyle s) -> Printer& {
        ctx_.push(s);
        return *this;
    }

    auto pop() -> Printer& {
        ctx_.pop();
        return *this;
    }

    auto reset() -> Printer& {
        ctx_.reset();
        return *this;
    }

    [[nodiscard]] auto current_style() const -> Style {
        return ctx_.current_abstyle().inner();
    }

    [[nodiscard]] auto base_style() const -> Style { return ctx_.base_style(); }

    /// Clones this `Printer` with different underlying stream.
    [[nodiscard]] auto clone_with(std::ostream& os) const -> Printer {
        return Printer(cfg_, &os, ctx_);
    }

    /// Clones this `Printer` with different underlying stream.
    [[nodiscard]] auto clone_with(std::FILE* f) const -> Printer {
        return Printer(cfg_, f, ctx_);
    }

    /* ----- print ----- */

    template <typename... Args>
    auto print(std::format_string<Args...> fmt, Args&&... args) -> Printer& {
        this->print_impl(null_style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto print(Style style, std::format_string<Args...> fmt, Args&&... args)
        -> Printer& {
        this->print_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto print(AbsoluteStyle style,
               std::format_string<Args...> fmt,
               Args&&... args) -> Printer& {
        this->print_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto println(std::format_string<Args...> fmt, Args&&... args) -> Printer& {
        this->println_impl(null_style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto println(Style style, std::format_string<Args...> fmt, Args&&... args)
        -> Printer& {
        this->println_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto println(AbsoluteStyle style,
                 std::format_string<Args...> fmt,
                 Args&&... args) -> Printer& {
        this->println_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

  private:
    using Stream = std::variant<detail::NotNull<std::FILE*>,
                                detail::NotNull<std::ostream*>>;

    explicit Printer(const OutputConfig* config,
                     Stream stream,
                     detail::StyleContext ctx)
        : ctx_(std::move(ctx)),
          cfg_(config),
          stream(stream) {
        ctx_.drop_pending();
        ctx_.ensure_style();
    }

    template <std::output_iterator<const char&> OutputIt,
              detail::style_type StyleT>
    auto write_pending(OutputIt out, StyleT with) -> OutputIt {
        assert(ctx_.has_pending());
        auto pending = ctx_.consume_pending();
        if (!with.is_null()) pending.merge(with);
        return pending.visit(
            [this, out](detail::style_type auto style) constexpr {
                return detail::write_style(out, style, cfg_);
            });
    }

    template <std::output_iterator<const char&> OutputIt,
              detail::style_type StyleT,
              typename... Args>
    auto format_to(OutputIt out,
                   StyleT style,
                   std::format_string<Args...> fmt,
                   Args&&... args /* NOLINT */) -> OutputIt {
        if (ctx_.has_pending()) out = this->write_pending(out, style);
        else out = detail::write_style(out, style, cfg_);

        // For some reason, GCC reports this variable as unused.
        [[maybe_unused]] const AbsoluteStyle current_style =
            detail::merge(ctx_.current_abstyle(), style);

        out = std::vformat_to(out,
                              fmt.get(),
                              detail::make_format_args(detail::process_fmt_arg(
                                  args, current_style, cfg_)...));

        if (!style.is_null())
            out = detail::write_style(out, ctx_.current_abstyle(), cfg_);

        return out;
    }

    template <detail::style_type StyleT, typename... Args>
    void print_impl(StyleT style,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
        auto buffer = detail::FormatBuf();
        this->format_to(detail::BufAppender(buffer),
                        style,
                        fmt,
                        std::forward<Args>(args)...);
        this->print_stream(buffer);
    }

    template <detail::style_type StyleT, typename... Args>
    void println_impl(StyleT style,
                      std::format_string<Args...> fmt,
                      Args&&... args) {
        auto buffer = detail::FormatBuf();
        this->format_to(detail::BufAppender(buffer),
                        style,
                        fmt,
                        std::forward<Args>(args)...);
        this->println_stream(buffer);
    }

    void print_stream(const detail::FormatBuf& buf) {
        if (auto* f = std::get_if<detail::NotNull<std::FILE*>>(&stream)) {
            std::print(*f, "{}", std::string_view(buf.data(), buf.size()));
        } else if (auto* os =
                       std::get_if<detail::NotNull<std::ostream*>>(&stream)) {
            std::print(**os, "{}", std::string_view(buf.data(), buf.size()));
        }
    }

    void println_stream(const detail::FormatBuf& buf) {
        if (auto* f = std::get_if<detail::NotNull<std::FILE*>>(&stream)) {
            std::println(*f, "{}", std::string_view(buf.data(), buf.size()));
        } else if (auto* os =
                       std::get_if<detail::NotNull<std::ostream*>>(&stream)) {
            std::println(**os, "{}", std::string_view(buf.data(), buf.size()));
        }
    }

    detail::StyleContext ctx_;

    const OutputConfig* cfg_;
    Stream stream;
};

/// Creates a `Printer` links to `stdout` and global output config.
inline auto stdout_printer() -> Printer { return Printer(config(), stdout); }

/// Creates a `Printer` links to `stderr` and global output config.
inline auto stderr_printer() -> Printer { return Printer(config(), stderr); }

#endif // DECO_ENABLE_STD_PRINT

} // namespace deco

#ifdef DECO_ENABLE_STD_PRINT
#undef DECO_ENABLE_STD_PRINT
#endif

#endif // !DECOTERM_FORMAT_HPP

