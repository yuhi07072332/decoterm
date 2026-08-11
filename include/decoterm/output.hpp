// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_OUTPUT_HPP
#define DECOTERM_OUTPUT_HPP

#include "style.hpp"

#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace deco {

namespace detail {

template <typename StyleT>
    requires std::is_same_v<std::remove_cvref_t<StyleT>, Style>
    || std::is_same_v<std::remove_cvref_t<StyleT>, AbsoluteStyle>
constexpr auto apply_style(AbsoluteStyle current, StyleT style)
    -> AbsoluteStyle {
    using style_type = std::remove_cvref_t<StyleT>;
    if constexpr (std::is_same_v<style_type, Style>)
        return abs(current.style | style);
    else if constexpr (std::is_same_v<style_type, AbsoluteStyle>) return style;
}

/* ----- global style output context ----- */

// NOLINTBEGIN

struct style_output_context_t {};

/// @brief global style output context for `StyleState`.
/// @details Used to determine whether `StyleOutputState` needs to output the
/// current style before outputting a value.
inline const style_output_context_t* g_style_output_context = nullptr;

// NOLINTEND

/* ----- StyleStack ----- */

/// @brief Represents a nullable single-or-multiple `AbsoluteStyle`
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
// ║                       StyleState                        ║
// ╚═════════════════════════════════════════════════════════╝

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
    StyleState() = default;
    StyleState(const StyleState&) = default;
    StyleState(StyleState&&) = default;
    auto operator=(const StyleState&) -> StyleState& = default;
    auto operator=(StyleState&&) -> StyleState& = default;

    ~StyleState() {
        if (context_tracking_enabled_ && detail::g_style_output_context == &context_)
            detail::g_style_output_context = nullptr;
    }

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
    auto context_tracking_enabled() const -> bool {
        return context_tracking_enabled_;
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
        stack_.push(abs(current_style().style | style));
    }

    /// @brief Updates the context if context tracking is enabled; otherwise only
    /// checks whether the base style changed.
    /// @returns The Style to emit when context or base style changed
    auto update_context() -> std::optional<AbsoluteStyle> {
        if (!context_tracking_enabled_) {
            if (base_style_changed_) {
                base_style_changed_ = false;
                return base_style_;
            }
            return std::nullopt;
        }
        if (detail::g_style_output_context != &context_) {
            detail::g_style_output_context = &context_;
            if (base_style_changed_) {
                base_style_changed_ = false;
                return abs(base_style_.style | current_style().style);
            }
            return current_style();
        }
        return std::nullopt;
    }

  private:
    template <typename>
    friend class StyleStateOption;

    AbsoluteStyle base_style_ = abs(null_style);
    detail::StyleStack stack_;

    bool style_enabled_ = true;
    bool context_tracking_enabled_ = false;

    bool base_style_changed_ = true;
    detail::style_output_context_t context_;
};

/// @brief CRTP class that provides chainable StyleState options
template <typename Derived>
class StyleStateOption {
  public:
    /// @brief Enable or disable style output.
    auto enable_style(bool enable = true) -> Derived& {
        state().style_enabled_ = enable;
        return underlying();
    }

    /// @brief Enable or disable context tracking.
    auto enable_context_tracking(bool enable = true) -> Derived& {
        state().context_tracking_enabled_ = enable;
        return underlying();
    }

    /// @brief Set the base style for this output state.
    auto set_base_style(Style base) -> Derived& {
        state().base_style_ = abs(base);
        state().base_style_changed_ = true;
        return underlying();
    }

    /// @brief Enable or disable style nesting.
    auto enable_nesting(bool enable = true) -> Derived& {
        if (enable) state().stack_.to_multiple();
        else state().stack_.to_single();
        return underlying();
    }

  private:
    friend Derived;

    StyleStateOption() = default;

    auto state() -> StyleState& {
        return static_cast<StyleState&>(static_cast<Derived&>(*this));
    }

    auto underlying() -> Derived& { return static_cast<Derived&>(*this); }
};

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledOstream                      ║
// ╚═════════════════════════════════════════════════════════╝

struct style_pop_t {};

/// @brief StyledOstream manipulator that restores the previous style.
/// @details If style nesting is not enabled, it will just restores to the base style.
inline constexpr style_pop_t pop;

/// @brief A stateful lightweight writer over an existing std::ostream.
/// @warning The passed std::ostream object must outlive this object.
/// @see `StyleState`
class StyledOstream : public StyleState,
                      public StyleStateOption<StyledOstream> {
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
        requires(!concepts::style<T> && !concepts::styled_ref<T>)
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
    template <concepts::styled_ref StyledRefT>
    friend auto operator<<(StyledOstream& out, StyledRefT&& styled) // NOLINT
        -> StyledOstream& {
        detail::check_styled_ref<StyledRefT>();
        out.ensure_context();

        out.output_style(styled.style());
        out.ostream() << styled.value();
        out.ostream() << out.current_style();
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           std::ios_base& (*fn)(std::ios_base&))
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           std::basic_ios<char>& (*fn)(std::basic_ios<char>&))
        -> StyledOstream& {
        out.ensure_context();
        out.ostream() << fn;
        return out;
    }

    /// @brief output operator for IO manipulators
    friend auto operator<<(StyledOstream& out,
                           std::ostream& (*fn)(std::ostream&))
        -> StyledOstream& {
        // `std::operator<<(std::ostream& os, std::ostream&(*fn)(std::ostream&))`
        // returns `fn(os)` instead of `os`, so we should assume that it may
        // return a different std::ostream&.
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
        if (style_enabled()) *ostream_ << style;
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
