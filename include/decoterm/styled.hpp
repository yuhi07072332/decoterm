// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_STYLED_HPP
#define DECOTERM_STYLED_HPP

#include "style.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace deco {

namespace detail {

template <typename T>
concept OstreamOutputable = requires(std::ostream& os, T&& value) {
    { os << std::forward<T>(value) } -> std::same_as<std::ostream&>;
};

/// @brief A class borrows an lvalue or owns an rvalue.
template <typename T>
struct Storage {
    constexpr explicit Storage(T& value)
        : value_(std::in_place_index<0>, std::addressof(value)) {}

    constexpr explicit Storage(T&& value)
        : value_(std::in_place_index<1>, std::move(value)) {}

    // A Ref is only movable.
    constexpr Storage(const Storage&) = delete;
    constexpr auto operator=(const Storage&) -> Storage& = delete;

    constexpr Storage(Storage&&) noexcept(
        std::is_nothrow_move_constructible_v<Variant>) = default;
    constexpr auto operator=(Storage&&) noexcept(
        std::is_nothrow_move_constructible_v<Variant>) -> Storage& = default;

    [[nodiscard]]
    constexpr auto operator*() const -> const T& {
        return get();
    }

    [[nodiscard]]
    constexpr auto get() const -> const T& {
        if (std::holds_alternative<0>(value_))
            return *std::get<0>(value_);
        else
            return std::get<1>(value_);
    }

  private:
    using Variant = std::variant<T*, T>;
    Variant value_;
};

/// @brief represents a nullable single or multiple(vector) AbsoluteStyle
/// stack.
class StyleStack {
  public:
    // single by default
    StyleStack() = default;

    [[nodiscard]]
    auto top() const -> std::optional<AbsoluteStyle> {
        if (is_multiple_) {
            if (multiple_.empty())
                return std::nullopt;
            else
                return multiple_.back();
        }
        return single_;
    }

    [[nodiscard]]
    auto is_multiple() const -> bool {
        return is_multiple_;
    }

    void push(AbsoluteStyle style) {
        if (is_multiple_) {
            multiple_.push_back(style);
        } else
            single_ = style;
    }

    void pop() {
        if (is_multiple_ && !multiple_.empty())
            multiple_.pop_back();
        else
            single_ = std::nullopt;
    }

    void clear() {
        if (is_multiple_)
            multiple_.clear();
        else
            single_ = std::nullopt;
    }

    void to_multiple() {
        if (is_multiple_) return;
        multiple_.clear();
        if (single_) multiple_.push_back(*single_);
        is_multiple_ = true;
    }

    void to_single() {
        if (!is_multiple_) return;
        if (!multiple_.empty())
            single_ = multiple_.back();
        else
            single_ = std::nullopt;
        is_multiple_ = false;
    }

  private:
    std::vector<AbsoluteStyle> multiple_;
    std::optional<AbsoluteStyle> single_;
    bool is_multiple_ = false;
};

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                        StyledRef                        ║
// ╚═════════════════════════════════════════════════════════╝


template <typename T, detail::OutputableStyle StyleType>
inline constexpr auto styled(T&& value, StyleType style) {
    return StyledRef(std::forward<T>(value), style);
}

template <typename T, detail::OutputableStyle StyleType>
auto operator<<(std::ostream& os, const StyledRef<T, StyleType>& styled_ref) {
    styled_ref.style.to_escape(std::ostreambuf_iterator(os));
    os << *styled_ref << reset;
    return os;
}

// ╔═════════════════════════════════════════════════════════╗
// ║                    StyleOutputState                     ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A class representing Style output state.
class StyleOutputState {
  public:
    StyleOutputState() = default;

    // ----- options -----

    auto enable_style(bool enable) -> StyleOutputState& {
        style_enabled_ = enable;
        return *this;
    }

    auto enable_color_fallback(bool enable) -> StyleOutputState& {
        color_fallback_enabled_ = enable;
        return *this;
    }

    auto base_style(Style base) -> StyleOutputState& {
        base_style_ = absolute(base);
        return *this;
    }

    auto enable_nesting(bool enable) -> StyleOutputState& {
        if (enable)
            stack_.to_multiple();
        else
            stack_.to_single();
        return *this;
    }

    // ----- observe -----

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
    auto color_fallback_enabled() const -> bool {
        return color_fallback_enabled_;
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
        stack_.push(absolute(current_style().style | style));
    }

    detail::StyleStack stack_;
    AbsoluteStyle base_style_ = absolute(default_style);

    bool style_enabled_ = true;
    bool color_fallback_enabled_ = false;
};

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledOstream                      ║
// ╚═════════════════════════════════════════════════════════╝

struct style_pop_t {};
inline constexpr style_pop_t pop;

/// @brief A stateful writer over an existing std::ostream.
/// @warning The passed std::ostream object must outlive this object.
class StyledOstream : public StyleOutputState {
  public:
    StyledOstream(std::ostream& os) : ostream_(os) {}

    template <detail::OstreamOutputable T>
    friend auto operator<<(StyledOstream& so, T&& value) -> StyledOstream& {
        using ValueType = std::remove_cvref_t<T>;
        if (so.is_first_output_) {
            so.output_style(so.base_style_);
            so.is_first_output_ = false;
        }

        if constexpr (detail::OutputableStyle<T>) {
            if constexpr (std::is_same_v<T, Style>
                          || std::is_same_v<T, AbsoluteStyle>)
                so.push_style(value);
            so.output_style(value);
        } else if constexpr (std::is_same_v<ValueType, style_reset_t>) {
            so.reset_style();
            so.output_style(so.current_style());
        } else {
            so.ostream_ << std::forward<T>(value);
        }
        return so;
    }

    friend auto operator<<(StyledOstream& so, style_pop_t) -> StyledOstream& {
        so.pop_style();
        if (so.style_enabled_) so.ostream_ << so.current_style();
        return so;
    }

    [[nodiscard]]
    auto ostream() -> std::ostream& { return ostream_; }

  private:
    template <detail::OutputableStyle StyleType>
    void output_style(StyleType style) {
        if (style_enabled_) ostream_ << style;
    }

    std::ostream& ostream_;

    bool is_first_output_ = true;
};

} // namespace deco

#endif // !DECOTERM_STYLED_HPP
