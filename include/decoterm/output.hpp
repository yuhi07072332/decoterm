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
#include <iostream>
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

/* ----- style merge ----- */

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

template <typename T, std::size_t N>
    requires(N > 0 && std::is_copy_assignable_v<T>)
class Buffer {
  public:
    using value_type = T;

    constexpr Buffer() = default;

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
    constexpr auto is_heap() const noexcept -> bool { return size_ > N; }

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
    }

    constexpr void grow() {
        const std::size_t new_capacity = capacity_ + (capacity_ / 2);
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
    const uint8_t dr = lhs.r - rhs.r;
    const uint8_t dg = lhs.g - rhs.g;
    const uint8_t db = lhs.b - rhs.b;
    return (2 * (dr * dr)) + (4 * (db * db)) + (3 * (dg * dg));
}

constexpr auto fallback_to_16(Color color) -> Color {
    const ColorType type = color.type();
    if (type == ColorType::Null || type == ColorType::DefaultColor)
        return color;

    RGB rgb; // NOLINT
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

    constexpr void merge_current_style_no_pending(detail::style_type auto style) {
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

class OutputConfig {
  public:
    OutputConfig() = default;

    auto enable_color(bool enable = true) -> OutputConfig& {
        color_enabled_.store(enable, std::memory_order_relaxed);
        return *this;
    }

    auto enable_style(bool enable = true) -> OutputConfig& {
        style_enabled_.store(enable, std::memory_order_relaxed);
        return *this;
    }

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

inline constinit OutputConfig global_cfg;       // NOLINT

// ╔═════════════════════════════════════════════════════════╗
// ║                         OStream                         ║
// ╚═════════════════════════════════════════════════════════╝

class OStream {
  public:
    explicit OStream(const OutputConfig& config, std::ostream& ostream)
        : cfg_(&config),
          ostream_(&ostream) {}

    auto set_base_style(Style s) -> OStream& {
        ctx_.set_base_style(s);
        return *this;
    }

    auto ensure_style() -> OStream& {
        ctx_.ensure_style();
        return *this;
    }

    [[nodiscard]] auto current_style() const -> Style {
        return ctx_.current_abstyle().inner();
    }

    [[nodiscard]] auto base_style() const -> Style { return ctx_.base_style(); }

    [[nodiscard]] auto ostream() const -> std::ostream& { return *ostream_; }

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
        out.ctx().merge_current_style(style);
        return out;
    }

    friend auto operator<<(OStream& out, AbsoluteStyle abstyle) -> OStream& {
        out.ctx().merge_current_style(abstyle);
        return out;
    }

    template <detail::style_type StyleT, ostream_outputable T>
    friend auto operator<<(OStream& out,
                           const detail::Styled<StyleT, T>& styled)
        -> OStream& {
        auto& ctx = out.ctx();
        if (ctx.has_pending()) {
            auto pending = ctx.consume_pending();
            pending.merge(styled.style());
            pending.visit([&out](auto style) { out.output_style(style); });
        } else {
            out.output_style(styled.style());
        }
        out.ostream() << styled.value();
        if (!styled.style().is_null()) out.output_style(ctx.current_abstyle());
        return out;
    }

    template <detail::style_type StyleT>
    friend auto operator<<(OStream& out, detail::Push<StyleT> push)
        -> OStream& {
        out.ctx().push(push.style);
        return out;
    }

    friend auto operator<<(OStream& out, detail::Pop) -> OStream& {
        out.ctx().pop();
        return out;
    }

    friend auto operator<<(OStream& out, detail::Reset) -> OStream& {
        out.ctx().reset();
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
    auto ctx() -> detail::StyleContext& { return ctx_; }

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
        if (!cfg_->style_enabled()) return;
        if (!cfg_->color_enabled()) style = detail::disable_color(style);
        else style = detail::fallback_color(style, cfg_->color_support());
        this->ostream() << style;
    }

    detail::StyleContext ctx_;

    detail::NotNull<const OutputConfig*> cfg_;
    detail::NotNull<std::ostream*> ostream_;
};

/// Creates a `OStream` that links to `std::cout` and global output config.
inline auto stdout_ostream() -> OStream {
    return OStream(global_cfg, std::cout);
}

/// Creates a `OStream` that links to `std::err` and global output config.
inline auto stderr_ostream() -> OStream {
    return OStream(global_cfg, std::cerr);
}

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
