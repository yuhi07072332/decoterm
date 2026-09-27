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
#include <memory>
#include <ostream>
#include <type_traits>
#include <utility>

namespace deco {

class OutputConfig;
class StyleState;

namespace detail {

constexpr auto merge(Style lhs, Style rhs) -> Style { return lhs | rhs; }

constexpr auto merge(AbsoluteStyle lhs, Style rhs) -> AbsoluteStyle {
    return abs(lhs.inner() | rhs);
}

constexpr auto merge(Style lhs, AbsoluteStyle rhs) -> AbsoluteStyle {
    return rhs;
}

constexpr auto merge(AbsoluteStyle lhs, AbsoluteStyle rhs) -> AbsoluteStyle {
    return rhs;
}

struct AnyStyle {
    constexpr AnyStyle() : type_(Type::Normal) {}
    constexpr AnyStyle(Style style) : type_(Type::Normal), inner_(style) {}
    constexpr AnyStyle(AbsoluteStyle abstyle)
        : type_(Type::Absolute),
          inner_(abstyle.inner()) {}

    constexpr void merge(detail::style_type auto s) {
        switch (type_) {
            case Type::Normal:
                *this = detail::merge(inner_, s);
                break;
            case Type::Absolute:
                *this = detail::merge(abs(inner_), s);
                break;
        }
    }

    template <typename Visitor>
    constexpr auto visit(Visitor&& visitor) const -> decltype(auto) { // NOLINT
        switch (type_) {
            case Type::Normal:
                return visitor(inner_);
            case Type::Absolute:
                return visitor(abs(inner_));
        }
    }

    constexpr auto is_null() const -> bool {
        return this->visit([](detail::style_type auto style){ return style.is_null(); });
    }

    constexpr auto inner() const -> Style { return inner_; }


  private:
    enum class Type { Normal, Absolute };

    Type type_;
    Style inner_;
};

template <detail::style_type StyleT>
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

template <std::size_t N>
    requires (N > 1)
class StyleStack {
  public:
    using value_type = AbsoluteStyle;

    constexpr StyleStack() = default;
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

    [[nodiscard]] constexpr auto top() -> value_type& {
        assert(!this->empty());
        return data_[size_ - 1];
    }

    constexpr void push(value_type v) {
        if (size_ == capacity_) this->grow();
        data_[size_++] = v; // NOLINT
    }

    constexpr void pop() { if (size_ > 0) size_--; }

    constexpr void clear() { size_ = 0; }

    [[nodiscard]] constexpr auto top_or(value_type v) const -> value_type {
        return this->empty() ? v : data_[size_ - 1];
    }

    [[nodiscard]] constexpr auto top() const -> value_type {
        assert(!this->empty());
        return data_[size_ - 1];
    }

    [[nodiscard]] constexpr auto empty() const -> bool { return size_; }

  private:
    constexpr auto is_heap() const noexcept -> bool {
        return size_ > N;
    }

    constexpr void copy_from(const StyleStack& other) {
        if (other.is_heap()) {
            heap_ = std::make_unique_for_overwrite<value_type[]>(
                other.capacity_);
            for (std::size_t i = 0; i < other.size_; ++i)
                heap_[i] = other.heap_[i];
            data_ = heap_.get();
        } else {
            local_ = other.local_;
            data_ = local_.data();
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
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

        other.size_ = 0;
    }

    constexpr void grow() {
        auto new_heap =
            std::make_unique_for_overwrite<value_type[]>(capacity_ * 2);
        for (std::size_t i = 0; i < size_; ++i)
            new_heap[i] = data_[i];
        heap_ = std::move(new_heap);
        data_ = heap_.get();
        capacity_ *= 2;
    }

    std::array<value_type, N> local_;
    std::unique_ptr<value_type[]> heap_;

    value_type* data_ = local_.data();
    std::size_t size_ = 0;
    std::size_t capacity_ = N;
};

/* ----- color fallback ----- */

using color_fallback_fn = Color (*)(Color);

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
    NotNull(Ptr ptr) : ptr_(ptr) {
        if (ptr == nullptr) throw std::logic_error("NotNull: null pointer");
    }

    auto operator->() const -> Ptr { return ptr_; }
    auto operator*() const -> decltype(auto) { return *ptr_; }

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

    void set_base_style(Style s) {
        AbsoluteStyle base = abs(s);
        if (stack_.empty())
            pending_.merge(base);
        base_style_ = base;
    }

    void ensure_style() {
        // TODO: needs to set active every output
        if (!this->needs_update_context()) return;
        detail::GlobalOutputContext::set_active(this);
        pending_.merge(this->current_abstyle());
    }

    [[nodiscard]] auto current_style() const -> Style {
        return this->current_abstyle().inner();
    }

    [[nodiscard]] auto base_style() const -> Style {
        return base_style_.inner();
    }

  protected:
    /// @details This will push `style` to the stack implicitly when the stack is empty.
    void set_current_style(detail::style_type auto style) {
        if (stack_.empty()) stack_.push(detail::merge(base_style_, style));
        else stack_.top() = detail::merge(stack_.top(), style);
        pending_.merge(style);
    }

    void push(detail::style_type auto style) {
        stack_.push(detail::merge(stack_.top_or(base_style_), style));
        pending_.merge(style);
    }

    void pop() {
        stack_.pop();
        pending_.merge(this->current_abstyle());
    }

    void reset() {
        stack_.clear();
        pending_.merge(this->current_abstyle());
    }

    [[nodiscard]] auto has_pending() -> bool { return !pending_.is_null(); }

    [[nodiscard]] auto consume_pending() -> detail::AnyStyle {
        auto pending = pending_;
        pending_ = null_style;
        return pending;
    }

    [[nodiscard]] auto needs_update_context() const -> bool {
        const StyleState* active = detail::GlobalOutputContext::get_active();
        return active != this && active
               && !(active->current_abstyle() == this->current_abstyle());
    }

    [[nodiscard]] auto current_abstyle() const -> AbsoluteStyle {
        return stack_.top_or(base_style_);
    }

  private:
    AbsoluteStyle base_style_;

    detail::StyleStack<3> stack_;
    detail::AnyStyle pending_;
};

// ╔═════════════════════════════════════════════════════════╗
// ║                         Ostream                         ║
// ╚═════════════════════════════════════════════════════════╝

class Ostream : public StyleState {
  public:
    explicit Ostream(OutputConfig& config, std::ostream& ostream)
        : cfg_(&config),
          ostream_(&ostream) {}

    auto ostream() const -> std::ostream& { return *ostream_; }

    /* ----- output operators ----- */

    /// @brief Output operator for any type that can be written to
    /// `std::ostream`.
    ///
    /// @throws `std::logic_error` if `operator<<(std::ostream&, T&&)` returns a
    /// different ostream object.
    template <ostream_outputable T>
        requires(!detail::style_type<T> && !detail::styled<T>)
    friend auto operator<<(Ostream& out, T&& value) -> Ostream& {
        out.output_pending();
        auto os_ptr = &(out.ostream() << std::forward<T>(value));
        if (os_ptr != out.ostream_)
            throw std::logic_error(
                "Ostream: std::ostream output operator returned a "
                "different ostream object");
        return out;
    }

    friend auto operator<<(Ostream& out, Style style) -> Ostream& {
        out.set_current_style(style);
        return out;
    }

    friend auto operator<<(Ostream& out, AbsoluteStyle abstyle) -> Ostream& {
        out.set_current_style(abstyle);
        return out;
    }

    template <detail::style_type StyleT, ostream_outputable T>
    friend auto operator<<(Ostream& out,
                           const detail::Styled<StyleT, T>& styled)
        -> Ostream& {
        if (out.has_pending()) {
            auto pending = out.consume_pending();
            pending.merge(styled.style());
            pending.visit([&out](auto style) {
                out.output_style(style);
            });
        } else {
            out.output_style(styled.style());
        }
        out.ostream() << styled.value();
        if (!styled.style().is_null()) out.output_style(out.current_abstyle());
    }

    template <detail::style_type StyleT>
    friend auto operator<<(Ostream& out, detail::Push<StyleT> push)
        -> Ostream& {
        out.push(push.style);
        return out;
    }

    friend auto operator<<(Ostream& out, detail::Pop) -> Ostream& {
        out.pop();
        return out;
    }

    friend auto operator<<(Ostream& out, detail::Reset) -> Ostream& {
        out.reset();
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(Ostream& out, std::ios_base& (*fn)(std::ios_base&))
        -> Ostream& {
        out.output_pending();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(Ostream& out,
                           std::basic_ios<char>& (*fn)(std::basic_ios<char>&))
        -> Ostream& {
        out.output_pending();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(Ostream& out, std::ostream& (*fn)(std::ostream&))
        -> Ostream& {
        // `std::operator<<(std::ostream& os,
        // std::ostream&(*fn)(std::ostream&))` returns `fn(os)` instead of `os`,
        // so we should assume that it may return a different std::ostream&.
        out.output_pending();
        out.ostream_ = &(out.ostream() << fn);
        return out;
    }

  private:
    void output_pending() {
        if (this->has_pending()) {
            const auto pending = this->consume_pending();
            pending.visit([this](detail::style_type auto style) constexpr {
                this->output_style(style);
            });
        }
    }

    template <detail::style_type StyleT>
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
