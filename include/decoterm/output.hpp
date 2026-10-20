// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see deco.hpp.

#ifndef DECOTERM_OUTPUT_HPP
#define DECOTERM_OUTPUT_HPP

#include "deco.hpp"

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifdef DECOTERM_ENABLE_STD_FORMAT
#   include <format>
#endif // DECOTERM_ENABLE_STD_FORMAT

#ifdef DECOTERM_ENABLE_STD_PRINT
#   include <cstdio>
#   include <print>
#   include <string_view>
#   include <variant>
#endif // DECOTERM_ENABLE_STD_PRINT

namespace deco {

class OutputConfig;

/// Terminal color capability levels.
enum class ColorSupport : uint8_t { Color16, Color256, TrueColor };

namespace detail {

struct RGB {
    uint8_t r, g, b;
};

// NOLINTBEGIN

// Generated from scripts/gen_color_names.py
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

#ifdef DECOTERM_ENABLE_STD_FORMAT

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

#endif // DECOTERM_ENABLE_STD_FORMAT

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

  protected:
    void switch_ostream(std::ostream& os) { ostream_ = &os; }

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

#ifdef DECOTERM_ENABLE_STD_FORMAT

namespace detail {

template <std::output_iterator<const char&> OutputIt, detail::style_type StyleT>
auto write_style(OutputIt out, StyleT style, const OutputConfig* cfg)
    -> OutputIt {
    if (!cfg) return style.to_escape(out);
    if (!cfg->style_enabled()) return out;
    if (!cfg->color_enabled()) style = detail::disable_color(style);
    else style = detail::fallback_color(style, cfg->color_support());
    return style.to_escape(out);
}

} // namespace detail

#ifdef DECOTERM_ENABLE_STD_PRINT

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

#endif // DECOTERM_ENABLE_STD_PRINT
#endif // DECOTERM_ENABLE_STD_FORMAT

} // namespace deco

#ifdef DECOTERM_ENABLE_STD_FORMAT

namespace std {

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

#endif // DECOTERM_ENABLE_STD_FORMAT

#endif // !DECOTERM_OUTPUT_HPP
