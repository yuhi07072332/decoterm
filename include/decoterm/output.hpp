// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_OUTPUT_HPP
#define DECOTERM_OUTPUT_HPP

#include "color_info.hpp"
#include "style.hpp"

#include <array>
#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>

namespace deco {

class OutputConfig;
class StyleState;

namespace detail {

template <detail::style StyleT>
struct Push {
    StyleT style;
};

struct Pop {};

struct GlobalOutputContext {
  public:
    static auto get_active() -> const StyleState* {
        return GlobalOutputContext::active().load(std::memory_order_relaxed);
    }

    static void set_active(const StyleState* active) {
        GlobalOutputContext::active().store(active, std::memory_order_relaxed);
    }

  private:
    static auto active() -> std::atomic<const StyleState*>& {
        static std::atomic<const StyleState*> active = nullptr;
        return active;
    }
};

/* ----- StyleStack ----- */

/// @brief
/// @details It has inline storage for N `AbsoluteStyle`s and grows to heap
/// if the size exceeds N.
template <std::size_t N>
class StyleStack {
    static_assert(N > 1);

  public:
    constexpr StyleStack() { local_[0] = AbsoluteStyle(); }

    constexpr StyleStack(const StyleStack& other) { this->copy_from(other); }
    constexpr StyleStack(StyleStack&& other) noexcept {
        this->move_from(std::move(other));
    }
    constexpr auto operator=(const StyleStack& other) -> StyleStack& {
        if (this == &other) return *this;
        this->copy_from(other);
        return *this;
    }
    constexpr auto operator=(StyleStack&& other) noexcept -> StyleStack& {
        if (this == &other) return *this;
        this->move_from(std::move(other));
        return *this;
    }

    constexpr ~StyleStack() = default;

    constexpr void push(AbsoluteStyle style) {
        if (is_heap_) {
            if (top_ == this->heap_end() - 1) this->grow();
            top_++;
        } else if (top_ == local_.end() - 1) {
            this->grow();
            top_ = heap_.get();
        } else {
            top_++;
        }

        *top_ = style;
    }

    constexpr void pop() {
        if (is_heap_) {
            if (top_ == heap_.get()) top_ = local_.data() + N - 1;
            else top_--;
        } else if (top_ > local_.data()) {
            top_--;
        }
    }

    constexpr void clear() {
        local_[0] = AbsoluteStyle();
        top_ = local_.data();
    }

    [[nodiscard]] constexpr auto top() -> AbsoluteStyle& { return *top_; }

    [[nodiscard]] constexpr auto top() const -> AbsoluteStyle { return *top_; }

  private:
    constexpr auto heap_end() const noexcept -> AbsoluteStyle* {
        return heap_.get() + heap_capacity();
    }

    constexpr auto heap_capacity() const noexcept -> std::size_t {
        return capacity_ > N ? capacity_ - N : 0;
    }

    constexpr void copy_from(const StyleStack& other) {
        local_ = other.local_;
        if (other.is_heap_) {
            std::size_t heap_size = other.top_ - other.heap_.get() + 1;
            heap_ = std::make_unique_for_overwrite<AbsoluteStyle[]>(
                other.heap_capacity());
            for (std::size_t i = 0; i < heap_size; ++i)
                heap_[i] = other.heap_[i];
            top_ = heap_.get() + heap_size - 1;
        } else {
            top_ = local_.data() + (other - other.local_.data());
        }
        capacity_ = other.capacity_;
        is_heap_ = other.is_heap_;
    }

    constexpr void move_from(StyleStack&& other) noexcept {
        local_ = other.local_;
        if (other.is_heap_) heap_ = std::move(other.heap_);

        top_ = other.top_;
        capacity_ = other.capacity_;
        is_heap_ = other.is_heap_;

        other.local_[0] = AbsoluteStyle();
        other.top_ = other.local_.data();
        other.capacity_ = N;
        other.is_heap_ = false;
    }

    constexpr void grow() {
        std::size_t heap_size = is_heap_ ? top_ - heap_.get() + 1 : 0;
        auto new_heap = std::make_unique_for_overwrite<AbsoluteStyle[]>(
            (capacity_ * 2) - N);
        for (std::size_t i = 0; i < heap_size; ++i)
            new_heap[i] = heap_[i];
        heap_ = std::move(new_heap);
        capacity_ *= 2;
    }

    std::array<AbsoluteStyle, N> local_;
    std::unique_ptr<AbsoluteStyle[]> heap_;

    AbsoluteStyle* top_ = local_.data();
    std::size_t capacity_ = N;
    bool is_heap_ = false;
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

constexpr auto fallback(AbsoluteStyle abstyle, color_fallback_fn fallback_fn)
    -> AbsoluteStyle {
    return abs(fallback(abstyle.inner(), fallback_fn));
}

template <typename Ptr>
    requires std::is_pointer_v<Ptr>
struct NotNull {
    using deref_type = decltype(*std::declval<Ptr>());

    NotNull(Ptr ptr) : ptr_(ptr) {
        if (ptr == nullptr)
            throw std::logic_error("NotNull: received null pointer");
    }

    auto operator->() const -> Ptr { return ptr_; }
    auto operator*() const -> deref_type& { return *ptr_; }

    operator Ptr() const { return ptr_; }

    auto get() const -> Ptr { return ptr_; }

  private:
    Ptr ptr_;
};

} // namespace detail

/// @brief Terminal color capability levels.
enum class ColorMode : uint8_t { color16 = 0, color256, true_color };

class OutputConfig {
  public:
    constexpr OutputConfig() = default;

    auto enable_style(bool enable = true) -> OutputConfig& {
        style_enabled_.store(enable, std::memory_order_relaxed);
        return *this;
    }

    auto set_color_mode(ColorMode mode) -> OutputConfig& {
        color_mode_.store(mode, std::memory_order_relaxed);
        return *this;
    }

    [[nodiscard]] auto style_enabled() const -> bool {
        return style_enabled_.load(std::memory_order_relaxed);
    }

    [[nodiscard]] auto color_mode() const -> ColorMode {
        return color_mode_.load(std::memory_order_relaxed);
    }

  private:
    std::atomic_bool style_enabled_ = true;
    std::atomic<ColorMode> color_mode_ = ColorMode::true_color;
};

class StyleState {
  public:
    StyleState() = default;
    StyleState(const StyleState&) = default;
    StyleState(StyleState&&) noexcept = default;
    auto operator=(const StyleState&) -> StyleState& = default;
    auto operator=(StyleState&&) noexcept -> StyleState& = default;

    ~StyleState() {
        if (detail::GlobalOutputContext::get_active() == this) {
            detail::GlobalOutputContext::set_active(nullptr);
        }
    }

    void set_current_style(Style style) {
        stack_.top() = abs(stack_.top().inner() | style);
    }

    void set_current_style(AbsoluteStyle abstyle) { stack_.top() = abstyle; }

    void push(Style style) { stack_.push(abs(stack_.top().inner() | style)); }

    void push(AbsoluteStyle abstyle) { stack_.push(abstyle); }

    [[nodiscard]] auto pop() -> AbsoluteStyle {
        stack_.pop();
        return stack_.top();
    }

    [[nodiscard]] auto reset() -> AbsoluteStyle {
        stack_.clear();
        return stack_.top();
    }

    [[nodiscard]] auto needs_update_context() const -> bool {
        const StyleState* active = detail::GlobalOutputContext::get_active();
        return active != this && active
               && !(active->current_style() == this->current_style());
    }

    [[nodiscard]] auto current_style() const -> AbsoluteStyle {
        return stack_.top();
    }

  private:
    detail::StyleStack<3> stack_;
};

// ╔═════════════════════════════════════════════════════════╗
// ║                         Ostream                         ║
// ╚═════════════════════════════════════════════════════════╝

class Ostream : public StyleState {
  public:
    explicit Ostream(OutputConfig& config, std::ostream& ostream)
        : cfg_(&config),
          ostream_(&ostream) {}

    void ensure_style() {
        if (!this->needs_update_context()) return;
        this->output_style(this->current_style());
        detail::GlobalOutputContext::set_active(this);
    }

    auto ostream() const -> std::ostream& { return *ostream_; }

    /* ----- output operators ----- */

    /// @brief Output operator for any type that can be written to
    /// `std::ostream`.
    ///
    /// @throws `std::logic_error` if `operator<<(std::ostream&, T&&)` returns a
    /// different ostream object.
    template <ostream_outputable T>
        requires(!detail::style<T> && !detail::styled<T>)
    friend auto operator<<(Ostream& out, T&& value) -> Ostream& {
        auto os_ptr = &(out.ostream() << std::forward<T>(value));
        if (os_ptr != out.ostream_)
            throw std::logic_error(
                "Ostream: std::ostream output operator returned a "
                "different ostream object");
        return out;
    }

    friend auto operator<<(Ostream& out, Style style) -> Ostream& {
        out.set_current_style(style);
        out.output_style(style);
        return out;
    }

    friend auto operator<<(Ostream& out, AbsoluteStyle abstyle) -> Ostream& {
        out.set_current_style(abstyle);
        out.output_style(abstyle);
        return out;
    }

    friend auto operator<<(Ostream& out, Reset) -> Ostream& {
        out.output_style(out.reset());
        return out;
    }

    template <detail::style StyleT, ostream_outputable T>
    friend auto operator<<(Ostream& out,
                           const detail::Styled<StyleT, T>& styled)
        -> Ostream& {
        out.output_style(styled.style());
        out.ostream() << styled.value();
        if (!styled.style().is_null()) out.output_style(out.current_style());
    }

    template <detail::style StyleT>
    friend auto operator<<(Ostream& out, detail::Push<StyleT> push)
        -> Ostream& {
        out.push(push.style);
        out.output_style(push.style);
        return out;
    }

    friend auto operator<<(Ostream& out, detail::Pop) -> Ostream& {
        out.output_style(out.pop());
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(Ostream& out,
                           auto (*fn)(std::ios_base&)->std::ios_base&)
        -> Ostream& {
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(
        Ostream& out, auto (*fn)(std::basic_ios<char>&)->std::basic_ios<char>&)
        -> Ostream& {
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(Ostream& out,
                           auto (*fn)(std::ostream&)->std::ostream&)
        -> Ostream& {
        // `std::operator<<(std::ostream& os,
        // std::ostream&(*fn)(std::ostream&))` returns `fn(os)` instead of `os`,
        // so we should assume that it may return a different std::ostream&.
        out.ostream_ = &(out.ostream() << fn);
        return out;
    }

  private:
    template <detail::style StyleT>
    void output_style(StyleT style) {
        if (!cfg_->style_enabled()) return;
        switch (cfg_->color_mode()) {
            case ColorMode::color16:
                style = detail::fallback(style, detail::fallback_to_16);
            case ColorMode::color256:
                style = detail::fallback(style, detail::fallback_to_256);
            default:
        }
        this->ostream() << style;
    }

    detail::NotNull<OutputConfig*> cfg_;
    detail::NotNull<std::ostream*> ostream_;
};

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
