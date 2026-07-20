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

template <typename T, detail::outputable_style StyleT> struct Styled;

class StyleOutputState;

namespace detail {

/* ----- global style output context ----- */

/// @brief global style output context for `StyleOutputState`.
///
/// Used to determine whether `StyleOutputState` needs to output the current
/// style before outputting a value.
inline const StyleOutputState* g_style_output_context = nullptr;

/* ----- type traits & concepts ----- */

// Whether `T` has `operator<<(std::ostream&, const T&)`
template <typename T>
concept ostream_outputable = requires(std::ostream& os, const T& value) {
    { os << value } -> std::same_as<std::ostream&>;
};

// Extract inner type from `std::reference_wrapper`.
// This can also check if `T` is a `std::reference_wrapper`.
template <typename T>
struct remove_reference_wrapper : std::false_type {
    using type = T;
};

template <typename T>
struct remove_reference_wrapper<std::reference_wrapper<T>> : std::true_type {
    using type = T;
};

template <typename T>
using remove_reference_wrapper_t = remove_reference_wrapper<T>::type;

template <typename T>
inline constexpr bool is_reference_wrapper_v =
    remove_reference_wrapper<T>::value;

// Whether `T` is a `Styled`.
template <typename T> struct is_styled : std::false_type {};

template <typename T, typename U>
struct is_styled<Styled<T, U>> : std::true_type {};

// Whether `remove_cvref_t<T>` is a `Styled`.
template <typename T>
concept styled = is_styled<std::remove_cvref_t<T>>::value;

/* ----- StyleStack ----- */

/// @brief represents a nullable, single or multiple `AbsoluteStyle`
/// stack.
class StyleStack {
  public:
    // single by default
    StyleStack() = default;

    [[nodiscard]]
    auto top() const -> std::optional<AbsoluteStyle> {
        if (is_multiple_) {
            if (multiple_.empty()) return std::nullopt;
            else return multiple_.back();
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
        } else single_ = style;
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
// ║                         Styled                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A wrapper for styling a value. Owns **const** rvalue or references
/// **const** lvalue.
template <typename T, detail::outputable_style StyleT>
struct Styled {
    using value_type =
        std::remove_const_t<detail::remove_reference_wrapper_t<T>>;

    constexpr Styled(T value, StyleT style)
        : value_(std::move(value)),
          style_(style) {}

    constexpr auto value() const -> const value_type& { return value_; }
    constexpr auto style() const -> StyleT { return style_; }

  private:
    T value_;
    StyleT style_;
};

/// @brief create a `Styled` from rvalue
template <typename T, detail::outputable_style StyleT>
    requires(!std::is_lvalue_reference_v<T>)
inline constexpr auto styled(T&& value, StyleT style)
    -> Styled<std::remove_const_t<T>, StyleT> {
    return Styled<std::remove_const_t<T>, StyleT>(std::move(value), style);
}

/// @brief create a `Styled` from const lvalue
template <typename T, detail::outputable_style StyleT>
inline constexpr auto styled(const T& value, StyleT style)
    -> Styled<std::reference_wrapper<const T>, StyleT> {
    return Styled(std::cref(value), style);
}

/// @brief output operator for Styled
template <detail::ostream_outputable T, detail::outputable_style StyleT>
inline auto operator<<(std::ostream& os, const Styled<T, StyleT>& styled)
    -> std::ostream& {
    os << styled.style() << styled.value() << reset;
    return os;
}

// ╔═════════════════════════════════════════════════════════╗
// ║                    StyleOutputState                     ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A class representing Style output state.
///
/// It is used to manage the current style output state(e.g. style enabled, base
/// style, etc.) It also checks `detail::g_style_output_context` to determine
/// whether it needs to output the current style before outputting a value.
///
/// @see `g_style_output_context`, `StyledOstream`
class StyleOutputState {
  public:
    StyleOutputState() = default;

    ~StyleOutputState() {
        if (detail::g_style_output_context == this)
            detail::g_style_output_context = nullptr;
    }

    // ----- options -----

    /// @brief Enable or disable style output.
    auto enable_style(bool enable) -> StyleOutputState& {
        style_enabled_ = enable;
        return *this;
    }

    /// @brief Enable or disable color fallback.
    auto enable_color_fallback(bool enable) -> StyleOutputState& {
        color_fallback_enabled_ = enable;
        return *this;
    }

    /// @brief Set the base style for this output state.
    auto base_style(Style base) -> StyleOutputState& {
        base_style_ = absolute(base);
        return *this;
    }

    /// @brief Enable or disable style nesting.
    auto enable_nesting(bool enable) -> StyleOutputState& {
        if (enable) stack_.to_multiple();
        else stack_.to_single();
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

    ///@brief Update the global style output context to this object.
    ///@returns `true` if the context was updated.
    auto update_context() const -> bool {
        if (detail::g_style_output_context != this) {
            detail::g_style_output_context = this;
            return true;
        }
        return false;
    }

    AbsoluteStyle base_style_ = absolute(default_style);
    bool style_enabled_ = true;
    bool color_fallback_enabled_ = false;

  private:
    detail::StyleStack stack_;
};

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledOstream                      ║
// ╚═════════════════════════════════════════════════════════╝

struct style_pop_t {};
inline constexpr style_pop_t pop;

/// @brief A stateful lightweight writer over an existing std::ostream.
/// @warning The passed std::ostream object must outlive this object.
class StyledOstream : public StyleOutputState {
  public:
    StyledOstream(std::ostream& os) : ostream_(&os) {}
    StyledOstream(const StyleOutputState& state, std::ostream& os)
        : StyleOutputState(state),
          ostream_(&os) {}

    /// @brief Output operator for any type that is outputable to
    /// `std::ostream`, except for 'Styled'.
    /// @throws `std::logic_error` if `operator<<(std::ostream, T&&)` returns
    /// different ostream object.
    template <detail::ostream_outputable T>
        requires(!detail::styled<T>)
    friend auto operator<<(StyledOstream& out, T&& value) -> StyledOstream& {
        using value_type = std::remove_cvref_t<T>;
        out.ensure_context();

        if constexpr (detail::outputable_style<value_type>) {
            if constexpr (std::is_same_v<value_type, Style>
                          || std::is_same_v<value_type, AbsoluteStyle>)
                out.push_style(value);
            out.output_style(value);
        } else if constexpr (std::is_same_v<value_type, style_reset_t>) {
            out.reset_style();
            out.output_style(out.current_style());
        } else {
            if (auto os_ptr = &(out.ostream() << std::forward<T>(value));
                os_ptr != out.ostream_)
                throw std::logic_error(
                    "StyledOstream: std::ostream output operator returns "
                    "different ostream object");
        }
        return out;
    }

    /// @brief output operator for `pop`
    friend auto operator<<(StyledOstream& out, style_pop_t) -> StyledOstream& {
        out.ensure_context();
        out.pop_style();
        if (out.style_enabled_) out.ostream() << out.current_style();
        return out;
    }

    /// @brief output operator for `Styled`
    template <detail::ostream_outputable T, detail::outputable_style StyleT>
    friend auto operator<<(StyledOstream& out, const Styled<T, StyleT>& styled)
        -> StyledOstream& {
        out.ensure_context();
        out.output_style(styled.style());
        out.ostream() << styled.value();
        out.ostream() << out.current_style();
        return out;
    }

    /// @brief output operator for IO manipulators.
    friend auto operator<<(StyledOstream& out,
                           std::ios_base& (*fn)(std::ios_base&))
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators.
    friend auto operator<<(StyledOstream& out,
                           std::basic_ios<char>& (*fn)(std::basic_ios<char>&))
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators.
    ///
    /// The overloaded
    /// `operator<<(std::ostream& os, std::ostream&(*fn)(std::ostream&))`
    /// returns `fn(os)` instead of `os`, so we should assume that it may
    /// return a different std::ostream&.
    friend auto operator<<(StyledOstream& out,
                           std::ostream& (*fn)(std::ostream&))
        -> StyledOstream& {
        out.ensure_context();
        out.ostream_ = &(out.ostream() << fn);
        return out;
    }

    void switch_ostream(std::ostream& os) { ostream_ = &os; }

    auto ostream() const -> std::ostream& { return *ostream_; }

  private:
    /// @brief Output style if style output is enabled.
    template <detail::outputable_style StyleT>
    void output_style(StyleT style) const {
        if (style_enabled_) *ostream_ << style;
    }

    /// @brief Ensure that the current style is outputted if the context has
    /// changed.
    void ensure_context() const {
        if (update_context()) output_style(current_style());
    }

    std::ostream* ostream_;
};

inline auto styled_out(std::ostream& os) -> StyledOstream {
    return StyledOstream(os);
}

} // namespace deco

#endif // !DECOTERM_STYLED_HPP
