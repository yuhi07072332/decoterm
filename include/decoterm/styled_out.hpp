// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECO_STYLED_OUT_HPP
#define DECO_STYLED_OUT_HPP

#include "style.hpp"

#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace deco {

namespace detail {

/* ----- global style output context ----- */

struct style_output_context_t {};

/// @brief global style output context for `StyleOutputState`.
///
/// Used to determine whether `StyleOutputState` needs to output the current
/// style before outputting a value.
inline const style_output_context_t* g_style_output_context = nullptr; // NOLINT

/* ----- type traits ----- */

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
// ║                    StyleOutputState                     ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A CRTP class representing Style output state.
///
/// It is used to manage the current style output state(e.g. style enabled, base
/// style, etc.) If context tracking is enabled, it will also check
/// `detail::g_style_output_context` to determine whether it needs to output the
/// current style before outputting a value.
///
/// @see `g_style_output_context`, `StyledOstream`
template <typename Derived>
class StyleOutputState { // NOLINT
  public:
    template <typename T>
    StyleOutputState(const StyleOutputState<T>& other)
        : base_style_(other.base_style_),
          stack_(other.stack_),
          style_enabled_(other.style_enabled_),
          context_enabled_(other.context_enabled_),
          is_first_output_(other.is_first_output_) {}

    template <typename T>
    StyleOutputState(StyleOutputState<T>&& other) 
        : base_style_(other.base_style_),
          stack_(std::move(other.stack_)),
          style_enabled_(other.style_enabled_),
          context_enabled_(other.context_enabled_),
          is_first_output_(other.is_first_output_) {}

    ~StyleOutputState() {
        if (detail::g_style_output_context == &context_)
            detail::g_style_output_context = nullptr;
    }

    // ----- options -----

    /// @brief Enable or disable style output.
    auto enable_style(bool enable = true) -> Derived& {
        style_enabled_ = enable;
        return static_cast<Derived&>(*this);
    }

    /// @brief Enable or disable context tracking.
    auto enable_context(bool enable = true) -> Derived& {
        context_enabled_ = enable;
        return static_cast<Derived&>(*this);
    }

    /// @brief Set the base style for this output state.
    auto base_style(Style base) -> Derived& {
        base_style_ = absolute(base);
        return static_cast<Derived&>(*this);
    }

    /// @brief Enable or disable style nesting.
    auto enable_nesting(bool enable = true) -> Derived& {
        if (enable) stack_.to_multiple();
        else stack_.to_single();
        return static_cast<Derived&>(*this);
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
    auto context_enabled() const -> bool {
        return context_enabled_;
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

    void ensure_context() {
        if (!context_enabled_) {
            if (is_first_output_) {
                output_style(base_style_);
                is_first_output_ = false;
            }
            return;
        }
        if (detail::g_style_output_context != &context_) {
            detail::g_style_output_context = &context_;
            is_first_output_ = false;
            output_style(current_style());
        }
    }

    template <detail::outputable_style StyleT>
    void output_style(StyleT style) const {
        static_cast<const Derived*>(this)->output_style_impl(style);
    }

  private:
    template <typename>
    friend class StyleOutputState;

    friend Derived;

    StyleOutputState() = default;

    AbsoluteStyle base_style_ = absolute(default_style);
    detail::StyleStack stack_;

    bool style_enabled_ = true;
    bool context_enabled_ = false;

    bool is_first_output_ = true;
    detail::style_output_context_t context_ {};
};

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledOstream                      ║
// ╚═════════════════════════════════════════════════════════╝

struct style_pop_t {};
inline constexpr style_pop_t pop;

/// @brief A stateful lightweight writer over an existing std::ostream.
/// @note The passed std::ostream object must outlive this object.
class StyledOstream : public StyleOutputState<StyledOstream> {
  public:
    StyledOstream(std::ostream& os) : ostream_(&os) {}

    template <typename Derived>
    StyledOstream(StyleOutputState<Derived> state, std::ostream& os)
        : StyleOutputState(std::move(state)),
          ostream_(&os) {}

    /// @brief Output operator for any type that is outputable to
    /// `std::ostream`.
    ///
    /// @throws `std::logic_error` if `operator<<(std::ostream, T&&)` returns
    /// different ostream object.
    template <detail::ostream_outputable T>
        requires(!detail::outputable_style<T> && !detail::styled<T>)
    friend auto operator<<(StyledOstream& out, T&& value) -> StyledOstream& {
        out.ensure_context();

        if (auto os_ptr = &(out.ostream() << std::forward<T>(value));
            os_ptr != out.ostream_)
            throw std::logic_error(
                "StyledOstream: std::ostream output operator returns "
                "different ostream object");
        return out;
    }

    /// @brief output operator for Style types
    template <detail::outputable_style StyleT>
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
        if (out.style_enabled()) out.ostream() << out.current_style();
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
    /// `std::operator<<(std::ostream& os, std::ostream&(*fn)(std::ostream&))`
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
    friend class StyleOutputState<StyledOstream>;

    void ensure_context() const {
        
    }

    void output_style_impl(detail::outputable_style auto style) const {
        if (style_enabled()) *ostream_ << style;
    }

    std::ostream* ostream_;
};

/// @brief equivalent to `StyledOstream(os)`
[[nodiscard]]
inline auto styled_out(std::ostream& os) -> StyledOstream {
    return {os};
}

} // namespace deco

#endif // !DECO_STYLED_OUT_HPP
