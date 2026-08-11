// ┌                                                         ┐
// │        THIS FILE IS AUTO GENERATED, DO NOT EDIT!        │
// └                                                         ┘

// Terminal styling library for C++20
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

enum class ColorType : uint8_t {
    null = 0,
    default_color,
    terminal_color,
    true_color
};

namespace concepts {

// clang-format off

// Whether remove_cvref_t<StyleT> is a Style or an AbsoluteStyle.
// The requires expression describes the common interface of both types.
template <typename StyleT>
concept style = 
    (std::is_same_v<std::remove_cvref_t<StyleT>, Style>
    || std::is_same_v<std::remove_cvref_t<StyleT>, AbsoluteStyle>)
    && requires(const std::remove_cvref_t<StyleT>& style,
                const std::remove_cvref_t<StyleT> other_style,
                char* out) {
    { style.is_null() } -> std::same_as<bool>;
    { style == other_style } -> std::same_as<bool>;
    { style != other_style } -> std::same_as<bool>;
    { style.to_escape(out) } -> std::same_as<char*>;
};

// clang-format on

// Whether `T` has `operator<<(std::ostream&, const T&)`
template <typename T>
concept ostream_outputable = requires(std::ostream& os, const T& value) {
    { os << value } -> std::same_as<std::ostream&>;
};

} // namespace concepts

namespace detail {

template <typename, concepts::style>
struct StyledRef;

} // namespace detail

namespace concepts {

template <typename T>
struct is_styled_ref : std::false_type {};

template <typename T, style StyleT>
struct is_styled_ref<detail::StyledRef<T, StyleT>> : std::true_type {};

template <typename T>
concept styled_ref = is_styled_ref<std::remove_cvref_t<T>>::value;

} // namespace concepts

namespace detail {

using ColorData = std::array<uint8_t, 3>;

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

template <std::output_iterator<const char&> OutputIt>
constexpr auto
color_to_sgr_params(OutputIt out, bool is_bg, ColorType type, ColorData data)
    -> OutputIt {
    switch (type) {
    case ColorType::null:
        return out;
    case ColorType::default_color: {
        if (is_bg) out = write_to(out, "49");
        else out = write_to(out, "39");
        return out;
    }
    case ColorType::terminal_color: {
        uint8_t index = data[0];
        if (index < 16) {
            out = write_to(out,
                           is_bg ? SGR_PARAM_BG[index] : SGR_PARAM_FG[index]);
        } else {
            out = write_to(out, is_bg ? "48;5;" : "38;5;");
            out = write_to(out, index);
        }
        return out;
    }
    case ColorType::true_color: {
        auto [r, g, b] = data;
        out = write_to(out, is_bg ? "48;2;" : "38;2;");
        out = write_to(out, r);
        *out++ = ';';
        out = write_to(out, g);
        *out++ = ';';
        out = write_to(out, b);
        return out;
    }
    default:
        return out;
    }
    return out;
}

/* ----- StyledRef ----- */

template <typename T, concepts::style StyleT>
struct StyledRef {
    using value_type = T;

    constexpr StyledRef(const T& value, StyleT style)
        : value_(value),
          style_(style) {}

    constexpr auto value() const -> const T& { return value_; }
    constexpr auto style() const -> StyleT { return style_; }

  private:
    const T& value_; // NOLINT
    StyleT style_;
};

template <concepts::styled_ref StyledRefT>
consteval void check_styled_ref() {
    static_assert(!std::is_lvalue_reference_v<StyledRefT>,
                  "deco::detail::StyledRef: Cannot pass StyledRef as an lvalue "
                  "reference.");
}

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                          Color                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A struct representing a nullable terminal color.
struct Color {
    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE = 19;

    /* ----- constructors ----- */

    static constexpr auto default_color() -> Color {
        return Color(ColorType::default_color, {0, 0, 0});
    }

    static constexpr auto null_color() -> Color {
        return Color(ColorType::null, {0, 0, 0});
    }

    /// @brief Create a color implicitly from an index in [0, 255]
    constexpr Color(uint8_t index)
        : type_(ColorType::terminal_color),
          data_({index, 0, 0}) {}

    constexpr explicit Color(uint8_t r, uint8_t g, uint8_t b)
        : type_(ColorType::true_color),
          data_({r, g, b}) {}

    constexpr auto operator==(const Color&) const -> bool = default;
    constexpr auto operator!=(const Color&) const -> bool = default;

    /// equivalent to `!is_null()`
    constexpr explicit operator bool() const { return !is_null(); }

    /// Whether this Color's type is `ColorType::null`
    constexpr auto is_null() const -> bool { return type_ == ColorType::null; }
    constexpr auto type() const -> ColorType { return type_; }

    /// @brief Returns the raw data(`uint8_t[3]`) stored in this Color.
    /// @details Usage:
    /// ```cpp
    /// Color col = rgb(32, 64, 128);
    /// if (col.type() == ColorType::true_color) {
    ///     auto [r, g, b] = col.data();
    ///     /* ... */
    /// } else if (col.type() == ColorType::terminal_color) {
    ///     uint8_t index = col.data()[0];
    ///     /* ... */
    /// }
    /// ```
    constexpr auto data() const -> detail::ColorData { return data_; }

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
        case ColorType::null:
            return "[null color]";
        case ColorType::default_color:
            return "[Default]";
        case ColorType::terminal_color: {
            std::string debug("[type=TerminalColor");
            return debug.append(", index=")
                .append(std::to_string(data_[0]))
                .append("]");
        }
        case ColorType::true_color: {
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

    constexpr Color(ColorType type, detail::ColorData data)
        : type_(type),
          data_(data) {}

    ColorType type_;
    detail::ColorData data_;
};

/// @brief Create a color from RGB
constexpr auto rgb(uint8_t r, uint8_t g, uint8_t b) -> Color {
    return Color(r, g, b);
}

/// @brief Create a color from 0xRRGGBB
/// @pre rgb <= 0xFFFFFF
constexpr auto rgb(uint32_t hex) -> Color {
    // clang-format off
    if (hex > 0xffffff) 
        throw std::invalid_argument("deco::rgb(): rgb > 0xffffff");
    return Color((hex >> 16) & 0xFF,
                    (hex >> 8) & 0xFF,
                    hex & 0xFF);
    // clang-format on
}

/// @brief Create a color from HSV
/// @param h [0, 360): Hue of the color
/// @param s [0, 255]: Saturation of the color
/// @param v [0, 255]: Value (brightness) of the color
[[nodiscard]]
inline DECO_CONSTEXPR_CMATH auto hsv(uint16_t h, uint8_t s, uint8_t v) // NOLINT
    -> Color {
    // clang-format off

    if (h < 0 || h >= 360) throw std::invalid_argument(
        "deco::hsv(): h is not in range [0, 360)");

    float hp = h / 60.f;                               //NOLINT
    float sp = s / 255.0f;                             //NOLINT
    float vp = v / 255.0f;                             //NOLINT

    float f = hp - std::floor(hp);
    uint8_t p = std::round(vp * (1 - sp) * 255);             //NOLINT
    uint8_t q = std::round(vp * (1 - (f * sp)) * 255);       //NOLINT
    uint8_t t = std::round(vp * (1 - ((1 - f) * sp)) * 255); //NOLINT

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

struct style_reset_t {};

/// @brief IO manipulator that resets the terminal style.
inline constexpr style_reset_t reset;

/// @brief A struct representing a nullable terminal style.
/// @details If a Style is null, to_escape() doesn't output any ANSI escape
/// code.
struct Style {
    // clang-format off
    enum Emphasis : uint8_t {                              //NOLINT
        none                = 0,
        bold                = 1 << 0,
        dim                 = 1 << 1,
        italic              = 1 << 2,
        underline           = 1 << 3,
        blink               = 1 << 4,
        invert              = 1 << 5,
        strikethrough       = 1 << 6,
        underline_double    = 1 << 7,
    };

    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE =
        (Color::MAX_ESCAPE_CODE_SIZE * 2) + 13 + 3;

    // clang-format on

    /// @brief creates a null style
    /// @details emphasis = 0, fg, bg = null_color
    constexpr Style() = default;

    constexpr Style(uint8_t emphasis, Color fg = null_color, Color bg = null_color)
        : fg_data_(fg.data_),
          bg_data_(bg.data_),
          color_types_((static_cast<uint8_t>(fg.type_) << 4)
                       | static_cast<uint8_t>(bg.type_)),
          emphasis_(emphasis) {}

    // ----- operators -----

    /// @brief combine two styles
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

    /// @brief equivalent to !is_null()
    constexpr explicit operator bool() const { return !is_null(); }

    // ----- observe -----

    constexpr auto emphasis() const -> uint8_t { return emphasis_; }
    constexpr auto fg() const -> Color { return {fg_type(), fg_data_}; }
    constexpr auto bg() const -> Color { return {bg_type(), bg_data_}; }

    constexpr auto is_null() const -> bool {
        return emphasis_ == none && fg_null() && bg_null();
    }

    // ----- output -----

    /// @brief write SGR parameters to output iterator
    /// @details format: "P1;P2;...;Pn"
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_sgr_params(OutputIt out) const -> OutputIt {
        using namespace detail;
        if (is_null()) return out;
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

        if (!emphasis_) return out;
        uint8_t current_flag = emphasis_;
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
        if (is_null()) return out;

        out = detail::write_to(out, "\x1b[");
        out = to_sgr_params(out);
        *out++ = 'm';
        return out;
    }

    /// @brief write ANSI escape code to a string
    [[nodiscard]]
    auto to_escape() const -> std::string {
        std::string esc;
        to_escape(std::back_inserter(esc));
        return esc;
    }

    [[nodiscard]]
    auto debug_string() const -> std::string {
        std::string debug = "[flags=";
        std::array<char, 8> buf {};
        for (std::size_t i = 0; i < buf.size(); ++i) {
            uint8_t bit = 1 << (buf.size() - i - 1);
            buf[i] = (emphasis_ & bit) ? '1' : '0';
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
        return fg_type() == ColorType::null;
    }
    constexpr auto bg_null() const -> bool {
        return bg_type() == ColorType::null;
    }

    detail::ColorData fg_data_ = {0, 0, 0};
    detail::ColorData bg_data_ = {0, 0, 0};

    // Bit-packed color types for fg and bg.
    // fg uses the upper 4 bits and bg uses the lower 4 bits.
    uint8_t color_types_ = 0;

    uint8_t emphasis_ = none;
};

/// @brief A Style wrapper that represents an absolute style
/// @details Style output normally has **additive semantics**, writing
/// `std::cout << style1 << style2` is roughly equivalent to writing
/// `std::cout << (style1 | style2)`. AbsoluteStyle instead will **reset**
/// to the inner style.
/// Example:
/// ```cpp
/// std::cout << italic << bold; // current style: italic | bold
/// std::cout << reset;
/// std::cout << italic << abs(bold); // current style: bold
/// ```
struct AbsoluteStyle {
    static constexpr std::size_t MAX_ESCAPE_CODE_SIZE =
        Style::MAX_ESCAPE_CODE_SIZE + 1;

    Style style;

    constexpr explicit AbsoluteStyle(Style style = Style()) : style(style) {}

    constexpr auto operator==(const AbsoluteStyle&) const -> bool = default;
    constexpr auto operator!=(const AbsoluteStyle&) const -> bool = default;

    constexpr auto is_null() const -> bool { return false; }

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

/// @brief create an AbsoluteStyle from a Style
constexpr auto abs(Style style) -> AbsoluteStyle {
    return AbsoluteStyle(style);
}

/// @brief create a Style with foreground and background colors
constexpr auto color(Color fg, Color bg) -> Style {
    return {Style::none, fg, bg};
}

/// @brief create a Style with foreground color
constexpr auto fg(Color fg) -> Style { return {Style::none, fg}; }

/// @brief create a Style with background color
constexpr auto bg(Color bg) -> Style { return {Style::none, null_color, bg}; }

/// @brief output operator for Style types e.g. Style, AbsoluteStyle
template <concepts::style StyleT>
inline auto operator<<(std::ostream& os, StyleT style) -> std::ostream& {
    std::array<char, StyleT::MAX_ESCAPE_CODE_SIZE> buf = {0};
    auto len = style.to_escape(buf.begin()) - buf.begin();
    os.write(buf.begin(), len);
    return os;
}

/// @brief output operator for deco::reset
inline auto operator<<(std::ostream& os, style_reset_t) -> std::ostream& {
    std::array<char, Style::MAX_ESCAPE_CODE_SIZE> buf = {0};
    auto len = abs(Style()).to_escape(buf.begin()) - buf.begin();
    os.write(buf.begin(), len);
    return os;
}

/* ----- style constants ----- */

// clang-format off

inline constexpr Style null_style        = Style();
inline constexpr Style bold              = Style(Style::bold);
inline constexpr Style dim               = Style(Style::dim);
inline constexpr Style italic            = Style(Style::italic);
inline constexpr Style underline         = Style(Style::underline);
inline constexpr Style blink             = Style(Style::blink);
inline constexpr Style invert            = Style(Style::invert);
inline constexpr Style strikethrough     = Style(Style::strikethrough);
inline constexpr Style underline_double  = Style(Style::underline_double);

// clang-format on

// ╔═════════════════════════════════════════════════════════╗
// ║                         Styled                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Wraps a value with a style for ostream and format output.
/// @details The `detail::StyledRef` has **reference semantics** and is intended
/// to be used only as a temporary. Example:
/// ```cpp
/// std::cout << styled(32, bold);  // OK
///
/// auto styled_int = styled(64, italic);
/// std::cout << styled_int;  // Bad: styled_int holds a dangling reference
/// ```
template <typename T, concepts::style StyleT>
constexpr auto styled(const T& value, StyleT style)
    -> detail::StyledRef<std::remove_cvref_t<T>, StyleT> {
    return detail::StyledRef(value, style);
}

/// @brief output operator for StyledRef
template <concepts::styled_ref StyledRefT>
inline auto operator<<(std::ostream& os, StyledRefT&& styled) // NOLINT
    -> std::ostream& {
    detail::check_styled_ref<StyledRefT>();
    static_assert(concepts::ostream_outputable<typename StyledRefT::value_type>,
                  "deco::StyledRef: T must be ostream outputable");

    os << styled.style() << styled.value() << reset;
    return os;
}

} // namespace deco

#undef DECO_CONSTEXPR_CMATH

#endif // !DECOTERM_STYLE_HPP


#ifndef DECOTERM_OUTPUT_HPP
#define DECOTERM_OUTPUT_HPP


#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace deco {

namespace detail {

template <typename StyleT>
    requires std::is_same_v<std::remove_cvref_t<StyleT>, Style>
    || std::is_same_v<std::remove_cvref_t<StyleT>, AbsoluteStyle>
constexpr auto apply_style(AbsoluteStyle current, StyleT style)
    -> AbsoluteStyle {
    using style_type = std::remove_cvref_t<StyleT>;
    if constexpr (std::is_same_v<style_type, Style>)
        return abs(current.style | style);
    else if constexpr (std::is_same_v<style_type, AbsoluteStyle>) return style;
}

/* ----- global style output context ----- */

// NOLINTBEGIN

struct style_output_context_t {};

/// @brief global style output context for `StyleState`.
/// @details Used to determine whether `StyleOutputState` needs to output the
/// current style before outputting a value.
inline const style_output_context_t* g_style_output_context = nullptr;

// NOLINTEND

/* ----- StyleStack ----- */

/// @brief Represents a nullable single-or-multiple `AbsoluteStyle`
/// stack.
class StyleStack {
  public:
    // single by default
    StyleStack() = default;

    [[nodiscard]]
    auto top() const -> std::optional<AbsoluteStyle> {
        if (is_multiple_) {
            if (multiple_.empty()) return std::nullopt;
            return multiple_.back();
        }
        return single_;
    }

    auto is_multiple() const -> bool { return is_multiple_; }

    void push(AbsoluteStyle style) {
        if (is_multiple_) multiple_.push_back(style);
        else single_ = style;
    }

    void pop() {
        if (is_multiple_ && !multiple_.empty()) multiple_.pop_back();
        else single_ = std::nullopt;
    }

    void clear() {
        if (is_multiple_) multiple_.clear();
        else single_ = std::nullopt;
    }

    void to_multiple() {
        if (is_multiple_) return;
        multiple_.clear();
        if (single_) multiple_.push_back(*single_);
        is_multiple_ = true;
    }

    void to_single() {
        if (!is_multiple_) return;
        if (!multiple_.empty()) single_ = multiple_.back();
        else single_ = std::nullopt;
        is_multiple_ = false;
    }

  private:
    std::vector<AbsoluteStyle> multiple_;
    std::optional<AbsoluteStyle> single_;
    bool is_multiple_ = false;
};

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                       StyleState                        ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A class representing style output state.
///
/// It manages the current style output state (e.g. style enabled, base
/// style, etc.) If context tracking is enabled, it will also check
/// `detail::g_style_output_context` to determine whether it needs to output the
/// current style before outputting a value.
///
/// @see `g_style_output_context`, `StyledOstream`, `StyledFormat`
class StyleState { // NOLINT
  public:
    StyleState() = default;
    StyleState(const StyleState&) = default;
    StyleState(StyleState&&) = default;
    auto operator=(const StyleState&) -> StyleState& = default;
    auto operator=(StyleState&&) -> StyleState& = default;

    ~StyleState() {
        if (context_tracking_enabled_ && detail::g_style_output_context == &context_)
            detail::g_style_output_context = nullptr;
    }

    [[nodiscard]]
    auto base_style() const -> Style {
        return base_style_.style;
    }

    [[nodiscard]]
    auto nesting_enabled() const -> bool {
        return stack_.is_multiple();
    }

    [[nodiscard]]
    auto style_enabled() const -> bool {
        return style_enabled_;
    }

    [[nodiscard]]
    auto context_tracking_enabled() const -> bool {
        return context_tracking_enabled_;
    }

    [[nodiscard]]
    auto current_style() const -> AbsoluteStyle {
        return stack_.top().value_or(base_style_);
    }

  protected:
    void pop_style() { stack_.pop(); }

    void reset_style() { stack_.clear(); }

    void push_style(AbsoluteStyle style) { stack_.push(style); }

    void push_style(Style style) {
        stack_.push(abs(current_style().style | style));
    }

    /// @brief Updates the context if context tracking is enabled; otherwise only
    /// checks whether the base style changed.
    /// @returns The Style to emit when context or base style changed
    auto update_context() -> std::optional<AbsoluteStyle> {
        if (!context_tracking_enabled_) {
            if (base_style_changed_) {
                base_style_changed_ = false;
                return base_style_;
            }
            return std::nullopt;
        }
        if (detail::g_style_output_context != &context_) {
            detail::g_style_output_context = &context_;
            if (base_style_changed_) {
                base_style_changed_ = false;
                return abs(base_style_.style | current_style().style);
            }
            return current_style();
        }
        return std::nullopt;
    }

  private:
    template <typename>
    friend class StyleStateOption;

    AbsoluteStyle base_style_ = abs(null_style);
    detail::StyleStack stack_;

    bool style_enabled_ = true;
    bool context_tracking_enabled_ = false;

    bool base_style_changed_ = true;
    detail::style_output_context_t context_;
};

/// @brief CRTP class that provides chainable StyleState options
template <typename Derived>
class StyleStateOption {
  public:
    /// @brief Enable or disable style output.
    auto enable_style(bool enable = true) -> Derived& {
        state().style_enabled_ = enable;
        return underlying();
    }

    /// @brief Enable or disable context tracking.
    auto enable_context_tracking(bool enable = true) -> Derived& {
        state().context_tracking_enabled_ = enable;
        return underlying();
    }

    /// @brief Set the base style for this output state.
    auto set_base_style(Style base) -> Derived& {
        state().base_style_ = abs(base);
        state().base_style_changed_ = true;
        return underlying();
    }

    /// @brief Enable or disable style nesting.
    auto enable_nesting(bool enable = true) -> Derived& {
        if (enable) state().stack_.to_multiple();
        else state().stack_.to_single();
        return underlying();
    }

  private:
    friend Derived;

    StyleStateOption() = default;

    auto state() -> StyleState& {
        return static_cast<StyleState&>(static_cast<Derived&>(*this));
    }

    auto underlying() -> Derived& { return static_cast<Derived&>(*this); }
};

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledOstream                      ║
// ╚═════════════════════════════════════════════════════════╝

struct style_pop_t {};

/// @brief StyledOstream manipulator that restores the previous style.
/// @details If style nesting is not enabled, it will just restores to the base style.
inline constexpr style_pop_t pop;

/// @brief A stateful lightweight writer over an existing std::ostream.
/// @warning The passed std::ostream object must outlive this object.
/// @see `StyleState`
class StyledOstream : public StyleState,
                      public StyleStateOption<StyledOstream> {
  public:
    StyledOstream(std::ostream& os) : ostream_(&os) {}

    StyledOstream(StyleState state, std::ostream& os)
        : StyleState(std::move(state)),
          ostream_(&os) {}

    /// @brief Output operator for any type that can be written to
    /// `std::ostream`.
    ///
    /// @throws `std::logic_error` if `operator<<(std::ostream&, T&&)` returns a
    /// different ostream object.
    template <concepts::ostream_outputable T>
        requires(!concepts::style<T> && !concepts::styled_ref<T>)
    friend auto operator<<(StyledOstream& out, T&& value) -> StyledOstream& {
        out.ensure_context();

        if (auto os_ptr = &(out.ostream() << std::forward<T>(value));
            os_ptr != out.ostream_)
            throw std::logic_error(
                "StyledOstream: std::ostream output operator returned a "
                "different ostream object");
        return out;
    }

    /// @brief output operator for Style types
    template <concepts::style StyleT>
    friend auto operator<<(StyledOstream& out, StyleT style) -> StyledOstream& {
        out.ensure_context();

        if constexpr (std::is_same_v<StyleT, Style>
                      || std::is_same_v<StyleT, AbsoluteStyle>)
            out.push_style(style);
        out.output_style(style);
        return out;
    }

    /// @brief output operator for `reset`
    friend auto operator<<(StyledOstream& out, style_reset_t)
        -> StyledOstream& {
        out.reset_style();
        out.output_style(out.current_style());
        return out;
    }

    /// @brief output operator for `pop`
    friend auto operator<<(StyledOstream& out, style_pop_t) -> StyledOstream& {
        out.ensure_context();
        out.pop_style();
        out.output_style(out.current_style());
        return out;
    }

    /// @brief output operator for styled values
    template <concepts::styled_ref StyledRefT>
    friend auto operator<<(StyledOstream& out, StyledRefT&& styled) // NOLINT
        -> StyledOstream& {
        detail::check_styled_ref<StyledRefT>();
        out.ensure_context();

        out.output_style(styled.style());
        out.ostream() << styled.value();
        out.ostream() << out.current_style();
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           std::ios_base& (*fn)(std::ios_base&))
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           std::basic_ios<char>& (*fn)(std::basic_ios<char>&))
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           std::ostream& (*fn)(std::ostream&))
        -> StyledOstream& {
        // `std::operator<<(std::ostream& os, std::ostream&(*fn)(std::ostream&))`
        // returns `fn(os)` instead of `os`, so we should assume that it may
        // return a different std::ostream&.
        out.ensure_context();
        out.ostream_ = &(out.ostream() << fn);
        return out;
    }

    /// @brief Swithes the underlying `std::ostream`.
    void set_stream(std::ostream& os) { ostream_ = &os; }

    /// @brief Returns the underlying `std::ostream`.
    auto ostream() const -> std::ostream& { return *ostream_; }

  private:
    void ensure_context() {
        if (auto style = update_context()) output_style(*style);
    }

    void output_style(concepts::style auto style) const {
        if (style_enabled()) *ostream_ << style;
    }

    std::ostream* ostream_;
};

/// @brief Creates a `StyledOstream` with context tracking enabled.
/// @details equivalent to `StyledOstream(os).enable_context()`
[[nodiscard]]
inline auto styled_out(std::ostream& os) -> StyledOstream {
    return StyledOstream(os).enable_context_tracking();
}

} // namespace deco

#endif // !DECOTERM_OUTPUT_HPP


#ifndef DECOTERM_TERMINAL_HPP
#define DECOTERM_TERMINAL_HPP


#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#else // POSIX
#include <unistd.h>

#endif

namespace deco {

/// @brief Terminal color capability levels.
enum class ColorSupport { true_color, color256, color16 };

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
inline auto get_color_support() -> ColorSupport {
#if defined(_WIN32)
    return ColorSupport::true_color;
#else // POSIX
    const char* colorterm_p = std::getenv("COLORTERM");

    std::string_view env_colorterm = colorterm_p ? colorterm_p : "";
    if (contains(env_colorterm, "truecolor")
        || contains(env_colorterm, "24bit"))
        return ColorSupport::true_color;

    // TODO:

    return ColorSupport::color16;
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
inline auto color_support() -> ColorSupport {
    static ColorSupport s_color_support = detail::get_color_support();
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


#ifndef DECOTERM_FORMAT_HPP
#define DECOTERM_FORMAT_HPP


#include <format>
#include <iterator>
#include <optional>
#include <type_traits>
#include <variant>
#include <version>

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202403L
#define DECO_ENABLE_PRINT
#include <print>
#endif

namespace deco {

namespace concepts {

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

} // namespace concepts

namespace detail {

struct StyledFormatContext {
    constexpr StyledFormatContext(AbsoluteStyle current_style,
                                  bool style_enabled)
        : current_style(current_style),
          style_enabled(style_enabled) {}

    AbsoluteStyle current_style;
    bool style_enabled;
};

// StyledRef with style context.
// This is used instead of StyledRef in StyledFormat.
template <typename StyledRefT>
struct FStyledRef {
    constexpr FStyledRef(StyledRefT styled, StyledFormatContext context)
        : styled(styled),
          context(context) {}

    StyledRefT styled;
    StyledFormatContext context; // NOLINT
};

// Replace StyledRef<T>& with FStyledRef<T>.
template <typename Arg>
constexpr auto process_arg(detail::StyledFormatContext context, Arg&& arg)
    -> decltype(auto) {
    if constexpr (concepts::styled_ref<Arg>) {
        return FStyledRef(arg, context);
    } else return std::forward<Arg>(arg); // NOLINT
}

template <typename Arg>
using processed_arg_t = std::conditional_t<concepts::styled_ref<Arg>,
                                           FStyledRef<std::remove_cvref_t<Arg>>,
                                           Arg>;

// this is used with process_arg(), since make_format_args don't take rvalue
// reference.
template <typename... Args>
constexpr auto make_format_args(Args&&... args /*NOLINT*/) {
    return std::make_format_args(args...);
}

} // namespace detail

} // namespace deco

// ╔═════════════════════════════════════════════════════════╗
// ║                       Formatters                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

/// @brief formatter for Style types, e.g. `Style` and `AbsoluteStyle`
template <deco::concepts::style StyleT>
struct formatter<StyleT> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(StyleT style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

/// @brief formatter for `deco::reset`
template <>
struct formatter<deco::style_reset_t> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::style_reset_t, std::format_context& ctx) const {
        return deco::abs(deco::null_style).to_escape(ctx.out());
    }
};

/// @brief formatter for styled values
template <deco::concepts::styled_ref StyledRefT>
    requires deco::concepts::formattable<typename StyledRefT::value_type>
struct formatter<StyledRefT> {
    std::formatter<typename StyledRefT::value_type, char> value_formatter;

    bool reset_on_end;

    constexpr formatter(bool reset_on_end = true)
        : reset_on_end(reset_on_end) {}

    constexpr auto parse(std::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    auto format(const StyledRefT& styled, /*NOLINT*/
                std::format_context& ctx) const {
        ctx.advance_to(styled.style().to_escape(ctx.out()));
        ctx.advance_to(value_formatter.format(styled.value(), ctx));
        if (reset_on_end)
            ctx.advance_to(
                formatter<deco::style_reset_t> {}.format(deco::reset, ctx));
        return ctx.out();
    }
};

// internal formatter
template <deco::concepts::styled_ref StyledRefT>
    requires deco::concepts::formattable<typename StyledRefT::value_type>
struct formatter<deco::detail::FStyledRef<StyledRefT>>
    : public formatter<StyledRefT> {
    using formatter<StyledRefT>::value_formatter;
    using formatter<StyledRefT>::parse;

    constexpr formatter() : formatter<StyledRefT>(false) {}

    auto format(const deco::detail::FStyledRef<StyledRefT>& fstyled, /*NOLINT*/
                std::format_context& ctx) const {
        if (fstyled.context.style_enabled)
            ctx.advance_to(fstyled.styled.style().to_escape(ctx.out()));
        ctx.advance_to(value_formatter.format(fstyled.styled.value(), ctx));
        if (fstyled.context.style_enabled)
            ctx.advance_to(fstyled.context.current_style.to_escape(ctx.out()));
        return ctx.out();
    }
};

}; // namespace std

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledFormat                       ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Format string type used by StyledFormat::print().
template <typename... Args>
using FormatString = std::format_string<detail::processed_arg_t<Args>...>;

/// @brief A stateful writer similar to `StyledOstream`, but with
/// `std::formatter` support.
/// @details Format arguments cannot contain Style types, reset, or pop. Use
/// print(style, ...), styled(), push(), reset(), or pop() instead.
class StyledFormat : public StyleState, public StyleStateOption<StyledFormat> {
    using stream_type = std::variant<std::FILE*, std::ostream*>;

  public:
    StyledFormat() = default;
    StyledFormat(StyleState state) : StyleState(std::move(state)) {}

    /* ----- Style operations ----- */

    /// Similar to `styled_os << style`, where `styled_os` is a StyledOstream
    /// object.
    auto push(concepts::style auto style) -> StyledFormat& {
        push_style(style);
        current_is_pending_ = true;
        return *this;
    }

    /// Similar to `styled_os << pop`, where `styled_os` is a StyledOstream
    /// object.
    auto pop() -> StyledFormat& {
        pop_style();
        current_is_pending_ = true;
        return *this;
    }

    /// Similar to `styled_os << reset`, where `styled_os` is a StyledOstream
    /// object.
    auto reset() -> StyledFormat& {
        reset_style();
        current_is_pending_ = true;
        return *this;
    }

    /* ----- format ----- */

    template <std::output_iterator<const char&> OutputIt,
              concepts::style StyleT,
              typename... Args>
    auto format_to(OutputIt out,
                   StyleT style,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt {
        (check_arg<Args>(), ...);
        out = ensure_context(out);
        const AbsoluteStyle current =
            detail::apply_style(current_style(), style);

        if (!style.is_null()) out = output_style(out, style);
        out = std::vformat_to(
            out,
            fmt.get(),
            detail::make_format_args(process_arg(
                detail::StyledFormatContext(current, style_enabled()),
                std::forward<Args>(args))...));
        if (!style.is_null()) out = output_style(out, current_style());
        return out;
    }

    template <std::output_iterator<const char&> OutputIt, typename... Args>
    auto format_to(OutputIt out,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt {
        return format_to(out, null_style, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(concepts::style auto style,
                std::format_string<Args...> fmt,
                Args&&... args) -> std::string {
        std::string buf;
        format_to(
            std::back_inserter(buf), style, fmt, std::forward<Args>(args)...);
        return buf;
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(std::format_string<Args...> fmt, Args&&... args)
        -> std::string {
        std::string buf;
        format_to(std::back_inserter(buf), fmt, std::forward<Args>(args)...);
        return buf;
    }

  private:
    template <typename Arg>
    static consteval void check_arg() {
        using arg_type = std::remove_cvref_t<Arg>;
        if constexpr (concepts::styled_ref<arg_type>)
            detail::check_styled_ref<Arg>();
        else
            static_assert(
                (!concepts::style<arg_type>
                 && !std::is_same_v<arg_type, style_reset_t>
                 && !std::is_same_v<arg_type, style_pop_t>),
                "deco::StyledFormat: Style, reset, pop are disallowed as "
                "format arguments. Use print(style, ...), styled(), "
                "push(), reset(), or pop() instead.");
    }

    template <std::output_iterator<const char&> OutputIt,
              concepts::style StyleT>
    auto output_style(OutputIt out, StyleT style) const -> OutputIt {
        if (style_enabled()) return style.to_escape(out);
        return out;
    }

    template <std::output_iterator<const char&> OutputIt>
    auto ensure_context(OutputIt out) -> OutputIt {
        const auto update = update_context();
        const AbsoluteStyle current = current_style();

        if (update) out = output_style(out, *update);
        if (current_is_pending_ && (!update || current != *update))
            out = output_style(out, current);
        current_is_pending_ = false;
        return out;
    }

    // Whether the current style needs to be emitted before the next output.
    bool current_is_pending_ = false;

    // ──────────────────────── std::print extensions ────────────────────────

#ifdef DECO_ENABLE_PRINT
  public:
    /* ----- print ----- */
    auto set_stream(FILE* f) -> StyledFormat& {
        stream_.emplace<FILE*>(f);
        return *this;
    }

    auto set_stream(std::ostream& os) -> StyledFormat& {
        stream_.emplace<std::ostream*>(&os);
        return *this;
    }

    /// @brief Prints with a temporary style to the stream.
    template <concepts::style StyleT, typename... Args>
    auto print(StyleT style, FormatString<Args...> fmt, Args&&... args)
        -> StyledFormat& {
        (check_arg<Args>(), ...);
        ensure_context();
        const AbsoluteStyle current =
            detail::apply_style(current_style(), style);

        if (!style.is_null()) output_style(style);
        print_stream(
            fmt,
            process_arg(detail::StyledFormatContext(current, style_enabled()),
                        std::forward<Args>(args))...);
        if (!style.is_null()) output_style(current_style());
        return *this;
    }

    template <typename... Args>
    auto print(FormatString<Args...> fmt, Args&&... args) -> StyledFormat& {
        print(null_style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    // TODO: println
  private:
    template <typename... Args>
    void print_stream(std::format_string<Args...> fmt, Args&&... args) const {
        if (std::holds_alternative<FILE*>(stream_)) {
            std::print(
                std::get<FILE*>(stream_), fmt, std::forward<Args>(args)...);
        } else if (std::holds_alternative<std::ostream*>(stream_)) {
            std::print(*std::get<std::ostream*>(stream_),
                       fmt,
                       std::forward<Args>(args)...);
        }
    }

    void output_style(concepts::style auto style) const {
        if (!style_enabled()) return;
        print_stream("{}", style);
    }

    void ensure_context() {
        const auto update = update_context();
        const AbsoluteStyle current = current_style();
        if (update) output_style(*update);
        if (current_is_pending_ && (!update || current != *update))
            output_style(current);
        current_is_pending_ = false;
    }

    stream_type stream_ = stream_type(std::in_place_type<FILE*>, stdout);

#endif // DECO_ENABLE_PRINT
};

#ifdef DECO_ENABLE_PRINT

/// @brief Creates a StyledFormat with context tracking enabled.
/// @details equivalent to
/// `StyledFormat().enable_context_tracking().set_stream(f)`
inline auto styled_fmt(std::FILE* f = stdout) -> StyledFormat {
    return StyledFormat().enable_context_tracking().set_stream(f);
}

/// @brief Creates a StyledFormat with context tracking enabled.
/// @details equivalent to
/// `StyledFormat().enable_context_tracking().set_stream(os)`
inline auto styled_fmt(std::ostream& os) -> StyledFormat {
    return StyledFormat().enable_context_tracking().set_stream(os);
}

#endif // DECO_ENABLE_PRINT

}; // namespace deco

#ifdef DECO_ENABLE_PRINT
#undef DECO_ENABLE_PRINT
#endif

#endif // !DECOTERM_FORMAT_HPP


#ifndef DECOTERM_COLOR_INFO_HPP
#define DECOTERM_COLOR_INFO_HPP

#include <cstdint>

// ╔═════════════════════════════════════════════════════════╗
// ║                       Color Info                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace deco::colors {

// NOLINTBEGIN
// clang-format off

/* ----- Terminal color names ----- */

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

#endif // !DECOTERM_COLOR_INFO_HPP

