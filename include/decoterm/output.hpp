// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_OUTPUT_HPP
#define DECOTERM_OUTPUT_HPP

#include "color_info.hpp"
#include "style.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>

namespace deco {

namespace detail {


/* ----- global style output context ----- */

// NOLINTBEGIN

struct style_output_context_t {};

/// @brief global style output context for `StyleState`.
/// @details Used to determine whether `StyleOutputState` needs to output the
/// current style before outputting a value.
inline const style_output_context_t* g_style_output_context = nullptr;

// NOLINTEND

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


/* ----- color fallback ----- */

using color_fallback_fn = auto (*)(Color) -> Color;

// TODO: replace this with better algorithm
constexpr auto rgb_distance(detail::RGB lhs, detail::RGB rhs) -> int {
    const uint8_t dr = lhs.r - rhs.r;
    const uint8_t dg = lhs.g - rhs.g;
    const uint8_t db = lhs.b - rhs.b;
    return (2 * (dr * dr)) + (4 * (db * db)) + (3 * (dg * dg));
}

constexpr auto fallback_to_16(Color color) -> Color {
    const ColorType type = color.type();
    if (type == ColorType::null || type == ColorType::default_color)
        return color;

    RGB rgb; // NOLINT
    if (type == ColorType::terminal_color) {
        const uint8_t index = color.data()[0];
        if (index < 16) return color;
        rgb = color_info[index];
    } else if (type == ColorType::true_color) {
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

constexpr auto fallback(Style style, color_fallback_fn fallback_fn) -> Style {
    return {style.emphasis(), fallback_fn(style.fg()), fallback_fn(style.bg())};
}

constexpr auto fallback(AbsoluteStyle style, color_fallback_fn fallback_fn)
    -> AbsoluteStyle {
    return abs(fallback(style.style, fallback_fn));
}

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                       StyleState                        ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Terminal color capability levels.
enum class ColorMode : uint8_t { color16 = 0, color256 = 1, true_color = 2 };

/// @brief Represents style output state.
///
/// It manages the current style output state (e.g. style enabled, base
/// style, etc.) If context tracking is enabled, it will also check
/// `detail::g_style_output_context` to determine whether it needs to output the
/// current style before outputting a value.
///
/// @see `g_style_output_context`, `StyledOstream`, `StyledFormat`
class StyleState { // NOLINT
  public:
    StyleState() {
        stack_.to_single();
    }

    StyleState(const StyleState& other) {
        if (!other.context_tracking_enabled_) try_clean_context();
        copy_from(other);
    };

    StyleState(StyleState&& other) noexcept {
        if (!other.context_tracking_enabled_) try_clean_context();
        move_from(std::move(other));
    }

    auto operator=(const StyleState& other) -> StyleState& {
        if (this == &other) return *this;
        if (!other.context_tracking_enabled_) try_clean_context();
        copy_from(other);
        return *this;
    };

    auto operator=(StyleState&& other) noexcept -> StyleState& {
        if (this == &other) return *this;
        if (!other.context_tracking_enabled_) try_clean_context();
        move_from(std::move(other));
        return *this;
    }

    ~StyleState() { try_clean_context(); }

    auto base_style() const -> Style { return stack_.base().style; }

    auto style_enabled() const -> bool { return style_enabled_; }

    auto nesting_enabled() const -> bool { return !stack_.is_single(); }

    auto color_mode() const -> ColorMode { return color_mode_; }

    auto context_tracking_enabled() const -> bool {
        return context_tracking_enabled_;
    }

    auto current_style() const -> AbsoluteStyle { return stack_.top(); }

  protected:
    void pop_style() { stack_.pop(); }

    void reset_style() { stack_.clear(); }

    void push_style(AbsoluteStyle style) { stack_.push(style); }

    void push_style(Style style) {
        stack_.push(abs(current_style().style | style));
    }

    /// @brief Updates the context if context tracking is enabled; otherwise
    /// only checks whether the base style changed.
    /// @returns The Style to emit when context or base style changed
    auto update_context() -> std::optional<AbsoluteStyle> {
        if (!context_tracking_enabled_) {
            if (base_style_changed_) {
                base_style_changed_ = false;
                return abs(base_style());
            }
            return std::nullopt;
        }
        if (detail::g_style_output_context != &context_) {
            detail::g_style_output_context = &context_;
            if (base_style_changed_) {
                base_style_changed_ = false;
                return abs(base_style() | current_style().style);
            }
            return current_style();
        }
        return std::nullopt;
    }

    void try_clean_context() const {
        if (!context_tracking_enabled_) return;
        if (detail::g_style_output_context == &context_)
            detail::g_style_output_context = nullptr;
    }

    template <concepts::style StyleT>
    auto fallback_style(StyleT style) const -> StyleT {
        switch (color_mode()) {
        case ColorMode::color16:
            return detail::fallback(style, detail::fallback_to_16);
        case ColorMode::color256:
            return detail::fallback(style, detail::fallback_to_256);
        default:
            return style;
        }
    }

  private:
    template <typename>
    friend class StyleStateSetter;

    void copy_from(const StyleState& other) {
        stack_ = other.stack_;
        style_enabled_ = other.style_enabled_;
        color_mode_ = other.color_mode_;
        context_tracking_enabled_ = other.context_tracking_enabled_;
        base_style_changed_ = other.base_style_changed_;
        context_ = other.context_;
    }

    void move_from(StyleState&& other) noexcept {
        stack_ = std::move(other.stack_);
        style_enabled_ = other.style_enabled_;
        color_mode_ = other.color_mode_;
        context_tracking_enabled_ = other.context_tracking_enabled_;
        base_style_changed_ = other.base_style_changed_;
        context_ = other.context_;
    }

    // config
    ColorMode color_mode_ = ColorMode::true_color;
    bool style_enabled_ = true;
    bool context_tracking_enabled_ = false;

    // state
    bool base_style_changed_ = true;
    detail::style_output_context_t context_;
    detail::StyleStack<5> stack_;
};

/// @brief CRTP class that provides chainable StyleState options
template <typename Self>
class StyleStateSetter {
  public:
    /// @brief Enable or disable style output.
    auto enable_style(bool enable = true) -> Self& {
        state().style_enabled_ = enable;
        return self();
    }

    auto enable_nesting(bool enable = true) -> Self& {
        if (!state().nesting_enabled() && enable) {
            state().stack_.to_multiple();
        } else if (state().nesting_enabled() && !enable) {
            state().stack_.to_single();
        }
        return self();
    }

    /// @brief Enable or disable context tracking.
    auto enable_context_tracking(bool enable = true) -> Self& {
        if (state().context_tracking_enabled_ && !enable) {
            state().try_clean_context();
        }
        state().context_tracking_enabled_ = enable;
        return self();
    }

    auto set_color_mode(ColorMode color_mode) -> Self& {
        state().color_mode_ = color_mode;
        return self();
    }

    /// @brief Set the base style for this output state.
    auto set_base_style(Style base) -> Self& {
        state().stack_.set_base(abs(base));
        state().base_style_changed_ = true;
        return self();
    }

  private:
    friend Self;

    StyleStateSetter() = default;

    auto state() -> StyleState& {
        return static_cast<StyleState&>(static_cast<Self&>(*this));
    }

    auto self() -> Self& { return static_cast<Self&>(*this); }
};

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledOstream                      ║
// ╚═════════════════════════════════════════════════════════╝

struct style_pop_t {};

/// @brief StyledOstream manipulator that restores the previous style.
inline constexpr style_pop_t pop;

/// @brief A stateful lightweight writer over an existing std::ostream.
/// @warning The passed std::ostream object must outlive this object.
/// @see `StyleState`
class StyledOstream : public StyleState,
                      public StyleStateSetter<StyledOstream> {
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
        requires(!concepts::style<T> && !detail::styled<T>)
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
    template <detail::styled StyledRefT>
    friend auto operator<<(StyledOstream& out, StyledRefT&& styled) // NOLINT
        -> StyledOstream& {
        out.ensure_context();

        out.output_style(styled.style());
        out.ostream() << styled.value();
        out.ostream() << out.current_style();
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           auto (*fn)(std::ios_base&)->std::ios_base&)
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(
        StyledOstream& out,
        auto (*fn)(std::basic_ios<char>&)->std::basic_ios<char>&)
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           auto (*fn)(std::ostream&)->std::ostream&)
        -> StyledOstream& {
        // `std::operator<<(std::ostream& os,
        // std::ostream&(*fn)(std::ostream&))` returns `fn(os)` instead of `os`,
        // so we should assume that it may return a different std::ostream&.
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
        if (!style_enabled()) return;
        ostream() << fallback_style(style);
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
