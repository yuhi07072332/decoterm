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
#include <cstring>
#include <iterator>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
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
                const std::remove_cvref_t<StyleT>& other_style,
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

template <concepts::style StyleT, typename... Ts>
    requires(!concepts::style<Ts> && ...)
struct Styled;

namespace detail {

template <typename T>
struct is_styled : std::false_type {};

template <concepts::style StyleT, typename... Ts>
struct is_styled<Styled<StyleT, Ts...>> : std::true_type {};

template <typename T>
concept styled = detail::is_styled<std::remove_cvref_t<T>>::value;

// Whether 'remove_cvref_t<T>' is a StyledRef.

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
constexpr auto color_to_sgr_params(OutputIt out,
                                   bool is_bg,
                                   ColorType type,
                                   ColorData data) -> OutputIt {
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

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                          Color                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Represents a nullable terminal color.
/// @details A `Color` is similar to a tagged union with 4 color types.
/// It stores a `ColorType` and a `uint8_t[3]` containing the data.
///
/// The meaning of each type:
/// * `null` => A null color. It emits no ANSI escape sequence.
///
/// * `default_color` => The terminal's default color.
///
/// * `terminal_color` => System 16 color or XTerm 256 color.
///     `data[0]` stores the index [0, 255], with indices 0-15 corresponding
///     to the system 16 colors.
///     (See https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit)
///
/// * `true_color` => A 24-bit RGB color.
///     `data[0]`, `data[1]`, `data[2]` store the red, green and blue components
///     respectively.
struct Color {
    static constexpr std::size_t MAX_ESCAPE_SEQ_SIZE = 19;

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
    constexpr auto data() const -> const detail::ColorData& { return data_; }

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

/// @brief Represents a nullable terminal style.
/// @details
/// * It contains 8 emphasis flags and 2 `Color`s for foreground color and
/// background color.
///
/// * A null 'Style' emits no ANSI escape sequence.
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

    static constexpr std::size_t MAX_ESCAPE_SEQ_SIZE =
        ((Color::MAX_ESCAPE_SEQ_SIZE - 3 + 1) * 2) + 14 + 3;

    // clang-format on

    constexpr Style(uint8_t emphasis = 0,
                    Color fg = null_color,
                    Color bg = null_color)
        : fg_data_(fg.data()),
          bg_data_(bg.data()),
          color_types_((static_cast<uint8_t>(fg.type()) << 4)
                       | static_cast<uint8_t>(bg.type())),
          emphasis_(emphasis) {}

    // ----- operators -----

    /// @brief Combines two styles.
    /// @details It works like applying 2 ANSI escape sequences, meaning that
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

    /// @brief
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

    /// @brief Writes SGR parameters to the output iterator.
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

    /// @brief Writes ANSI escape sequence to the output iterator.
    template <std::output_iterator<const char&> OutputIt>
    constexpr auto to_escape(OutputIt out) const -> OutputIt {
        if (is_null()) return out;

        out = detail::write_to(out, "\x1b[");
        out = to_sgr_params(out);
        *out++ = 'm';
        return out;
    }

    /// @brief Writes ANSI escape sequence to a string.
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

/// @brief A Style wrapper that represents an absolute style.
/// @details Style output normally has *additive semantics*, but this does't
/// have.
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

/// @brief Creates an AbsoluteStyle from a Style.
/// @details equivalent to `AbsoluteStyle(style)`
constexpr auto abs(Style style) -> AbsoluteStyle {
    return AbsoluteStyle(style);
}

/// @brief Creates a Style with foreground and background colors.
constexpr auto color(Color fg, Color bg) -> Style {
    return {Style::Emphasis::none, fg, bg};
}

/// @brief Create a Style with foreground color.
constexpr auto fg(Color fg) -> Style { return {Style::Emphasis::none, fg}; }

/// @brief Create a Style with background color.
constexpr auto bg(Color bg) -> Style {
    return {Style::Emphasis::none, null_color, bg};
}

/// @brief output operator for Style types e.g. Style, AbsoluteStyle
template <concepts::style StyleT>
inline auto operator<<(std::ostream& os, StyleT style) -> std::ostream& {
    std::array<char, StyleT::MAX_ESCAPE_SEQ_SIZE> buf = {0};
    auto len = style.to_escape(buf.begin()) - buf.begin();
    os.write(buf.begin(), len);
    return os;
}

/// @brief output operator for deco::reset
inline auto operator<<(std::ostream& os, style_reset_t) -> std::ostream& {
    std::array<char, Style::MAX_ESCAPE_SEQ_SIZE> buf = {0};
    auto len = abs(Style()).to_escape(buf.begin()) - buf.begin();
    os.write(buf.begin(), len);
    return os;
}

/* ----- style constants ----- */

// clang-format off

inline constexpr Style null_style        = Style();
inline constexpr Style bold              = Style(Style::Emphasis::bold);
inline constexpr Style dim               = Style(Style::Emphasis::dim);
inline constexpr Style italic            = Style(Style::Emphasis::italic);
inline constexpr Style underline         = Style(Style::Emphasis::underline);
inline constexpr Style blink             = Style(Style::Emphasis::blink);
inline constexpr Style invert            = Style(Style::Emphasis::invert);
inline constexpr Style strikethrough     = Style(Style::Emphasis::strikethrough);
inline constexpr Style underline_double  = Style(Style::Emphasis::underline_double);

// clang-format on

// ╔═════════════════════════════════════════════════════════╗
// ║                         Styled                          ║
// ╚═════════════════════════════════════════════════════════╝

namespace detail {

template <concepts::style StyleT>
constexpr auto apply_style(AbsoluteStyle current, StyleT style)
    -> AbsoluteStyle {
    using style_type = std::remove_cvref_t<StyleT>;
    if constexpr (std::is_same_v<style_type, Style>)
        return abs(current.style | style);
    else if constexpr (std::is_same_v<style_type, AbsoluteStyle>) return style;
}

/* ----- StyleStack ----- */

/// @brief A stack that can store single or multiple `AbsoluteStyle`.
/// @details It has inline storage for N `AbsoluteStyle`s and grows to heap
/// if the size exceeds N.
/// In single mode, the size will only be 0 or 1, and pushing style only
/// changes first element.
template <std::size_t N = 5>
class StyleStack {
    static_assert(N > 1);

  public:
    // multiple by default
    constexpr StyleStack() = default;
    constexpr StyleStack(const StyleStack& other) { copy_from(other); }
    constexpr StyleStack(StyleStack&& other) noexcept {
        move_from(std::move(other));
    }

    constexpr auto operator=(const StyleStack& other) -> StyleStack& {
        if (this == &other) return *this;
        copy_from(other);
        return *this;
    }

    constexpr auto operator=(StyleStack&& other) noexcept -> StyleStack& {
        if (this == &other) return *this;
        move_from(std::move(other));
        return *this;
    }

    constexpr ~StyleStack() = default;

    constexpr auto base() const -> AbsoluteStyle { return base_; }
    constexpr auto is_single() const -> bool { return is_single_; }

    constexpr auto top() const -> AbsoluteStyle {
        if (size_) return data_[size_ - 1];
        return base_;
    }

    constexpr void push(AbsoluteStyle style) {
        if (size_ == capacity_) {
            grow();
        } else if (is_single_) {
            size_ = 1;
            data_[0] = style;
            return;
        }
        data_[size_++] = style; // NOLINT
    }

    constexpr void pop() {
        if (size_) size_--;
    }

    constexpr void clear() { size_ = 0; }

    constexpr void set_base(AbsoluteStyle base) { base_ = base; }

    constexpr void to_single() {
        if (size_) {
            local_[0] = top();
            size_ = 1;
        }
        data_ = local_.data();
        is_single_ = true;
    }

    constexpr void to_multiple() { is_single_ = false; }

  private:
    constexpr auto is_heap() const noexcept -> bool {
        return data_ != local_.data();
    }

    constexpr void copy_from(const StyleStack& other) {
        if (other.is_heap()) {
            heap_ = std::make_unique_for_overwrite<AbsoluteStyle[]>(
                other.capacity_);
            std::memcpy(heap_.get(),
                        other.heap_.get(),
                        sizeof(AbsoluteStyle) * other.size_);
            data_ = heap_.get();
        } else {
            local_ = other.local_;
            data_ = local_.data();
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
        base_ = other.base_;
        is_single_ = other.is_single_;
    }

    constexpr void move_from(StyleStack&& other) noexcept {
        if (other.is_heap()) {
            heap_ = std::move(other.heap_);
            data_ = heap_.get();
        } else {
            local_ = other.local_;
            data_ = local_.data();
        }

        size_ = other.size_;
        capacity_ = other.capacity_;
        base_ = other.base_;
        is_single_ = other.is_single_;

        other.size_ = 0;
        other.capacity_ = N;
        other.data_ = other.local_.data();
    }

    constexpr void grow() {
        auto new_heap =
            std::make_unique_for_overwrite<AbsoluteStyle[]>(capacity_ * 2);
        std::memcpy(new_heap.get(), data_, sizeof(AbsoluteStyle) * size_);
        heap_ = std::move(new_heap);
        data_ = heap_.get();
        capacity_ *= 2;
    }

    std::array<AbsoluteStyle, N> local_;
    std::unique_ptr<AbsoluteStyle[]> heap_;

    AbsoluteStyle* data_ = local_.data();
    std::size_t size_ = 0;
    std::size_t capacity_ = N;

    AbsoluteStyle base_ = AbsoluteStyle();
    bool is_single_ = false;
};

template <typename T>
struct ConstRef {
    constexpr ConstRef(const T& ref) : ref(ref) {}

    constexpr auto get() const -> const T& { return ref; }
    constexpr operator const T&() const { return ref; }

  private:
    const T& ref; // NOLINT
};

template <typename T>
struct unwrap_constref {
    using type = T;
};

template <typename T>
struct unwrap_constref<ConstRef<T>> {
    using type = const T&;
};

template <typename T>
constexpr auto make_styled_child(const T& value) -> detail::ConstRef<T> {
    return detail::ConstRef<T>(value);
}

template <typename T>
    requires(!std::is_lvalue_reference_v<T>)
constexpr auto make_styled_child(T&& value) -> std::remove_cvref_t<T> {
    return std::forward<T>(value);
}

template <typename T>
constexpr auto make_styled_child(T* value) -> const T* {
    return value;
}

template <detail::styled Styled>
constexpr auto make_styled_child(Styled& styled) -> Styled& {
    return styled;
}

/// @param current_style The style reset to after the styled emitted.
template <typename OutputFn, concepts::style StyleT, typename... Ts>
constexpr void emit_styled(AbsoluteStyle current_style,
                           const OutputFn& output_fn /*NOLINT*/,
                           const Styled<StyleT, Ts...>& styled) {
    AbsoluteStyle applied_style =
        detail::apply_style(current_style, styled.style());
    auto emit_one = [&, applied_style]<typename P>(const P& value) {
        if constexpr (detail::styled<P>) {
            emit_styled(applied_style, output_fn, value);
        } else {
            output_fn(static_cast<detail::unwrap_constref<P>::type>(value));
        }
    };

    if (applied_style != current_style) output_fn(applied_style);
    std::apply(
        [&]<typename... Ps>(const Ps&... values) { (emit_one(values), ...); },
        styled.values());
    output_fn(current_style);
}

} // namespace detail

template <concepts::style StyleT, typename... Ts>
    requires(!concepts::style<Ts> && ...)
struct Styled {
    static_assert(sizeof...(Ts) >= 1, "Styled must contains at least 1 value");

    constexpr Styled(StyleT style, Ts... values)
        : style_(style),
          values_(std::move(values)...) {}

    constexpr auto style() const -> StyleT { return style_; }
    constexpr auto values() const -> const std::tuple<Ts...>& {
        return values_;
    }

  private:
    StyleT style_;
    std::tuple<Ts...> values_;
};

template <concepts::style StyleT, typename... Ts>
    requires(!concepts::style<Ts> && ...)
constexpr auto styled(StyleT style, Ts&&... value) {
    return Styled(style, detail::make_styled_child(std::forward<Ts>(value))...);
}

template <concepts::style StyleT, typename... Ts>
inline auto operator<<(std::ostream& os, const Styled<StyleT, Ts...>& styled)
    -> std::ostream& {
    detail::emit_styled(
        abs(null_style), [&](const auto& value) { os << value; }, styled);
    return os;
}

} // namespace deco

#undef DECO_CONSTEXPR_CMATH

#endif // !DECOTERM_STYLE_HPP
