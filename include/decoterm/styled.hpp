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
#include <functional>
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace deco {

namespace detail {

// Extract InnerType from std::reference_wrapper<InnerType>.
// This can also check if T is a std::reference_wrapper.
template <typename T>
struct remove_reference_wrapper : std::false_type {
    using type = T;
};

template <typename Inner>
struct remove_reference_wrapper<std::reference_wrapper<Inner>>
    : std::true_type {
    using type = Inner;
};

template <typename T>
using remove_reference_wrapper_t = remove_reference_wrapper<T>::type;

template <typename T>
inline constexpr bool is_reference_wrapper_v =
    remove_reference_wrapper<T>::value;

// Whether T has operator<<(std::ostream&, T)
template <typename T>
concept ostream_outputable = requires(std::ostream& os, T&& value) {
    { os << std::forward<T>(value) } -> std::same_as<std::ostream&>;
};

template <typename T, detail::outputable_style StyleType>
struct StyledValue {
    using ValueType = remove_reference_wrapper_t<T>;

    constexpr explicit StyledValue(T value, StyleType style)
        : value_(std::move(value)),
          style_(style) {}

    constexpr auto value() -> ValueType& { 
        if constexpr (is_reference_wrapper_v<T>) return value_.get();
        return value_; 
    }
    constexpr auto value() const -> const ValueType& { 
        if constexpr (is_reference_wrapper_v<T>) return value_.get();
        return value_; 
    }
    constexpr auto style() const -> StyleType { return style_; }

  private:
    T value_;
    StyleType style_;
};

/// @brief represents a nullable, single or multiple(vector) AbsoluteStyle
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
// ║                         Styled                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief create StyledValue from lvalue reference.
template <typename T, detail::outputable_style StyleType>
inline constexpr auto styled(T& value, StyleType style)
    -> detail::StyledValue<std::reference_wrapper<T>, StyleType> {
    return detail::StyledValue(std::ref(value), style);
}

/// @brief create StyledValue from rvalue.
template <typename T, detail::outputable_style StyleType>
inline constexpr auto styled(T&& value, StyleType style)
    -> detail::StyledValue<std::remove_cvref_t<T>, StyleType> {
    // This overloaded version must only take rvalue reference.
    static_assert(std::is_rvalue_reference_v<T&&>);
    return detail::StyledValue(std::move(value), style);
}

/// @brief ostream operator for styled()
template <typename T, detail::outputable_style StyleType>
auto operator<<(std::ostream& os,
                const detail::StyledValue<T, StyleType>& styled)
    -> std::ostream& {
    styled.style().to_escape(std::ostreambuf_iterator(os));
    os << styled.value() << reset;
    return os;
}

/// @brief ostream operator for styled()
template <typename T, detail::outputable_style StyleType>
auto operator<<(std::ostream& os,
                detail::StyledValue<T, StyleType>& styled)
    -> std::ostream& {
    styled.style().to_escape(std::ostreambuf_iterator(os));
    os << styled.value() << reset;
    return os;
}

/// @brief ostream operator for styled()
template <typename T, detail::outputable_style StyleType>
auto operator<<(std::ostream& os,
                detail::StyledValue<T, StyleType>&& styled)
    -> std::ostream& {
    styled.style().to_escape(std::ostreambuf_iterator(os));
    os << styled.value() << reset;
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

    template <detail::ostream_outputable T>
    friend auto operator<<(StyledOstream& so, T&& value) -> StyledOstream& {
        using ValueType = std::remove_cvref_t<T>;
        if (so.is_first_output_) {
            so.output_style(so.base_style_);
            so.is_first_output_ = false;
        }

        if constexpr (detail::outputable_style<T>) {
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
    auto ostream() const -> std::ostream& {
        return ostream_;
    }

  private:
    template <detail::outputable_style StyleType>
    void output_style(StyleType style) {
        if (style_enabled_) ostream_ << style;
    }

    std::ostream& ostream_;

    bool is_first_output_ = true;
};

} // namespace deco

#endif // !DECOTERM_STYLED_HPP
