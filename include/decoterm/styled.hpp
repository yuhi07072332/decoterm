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
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace deco {

namespace detail {

template <typename T>
concept OstreamOutputable = requires(std::ostream& os, T&& value) {
    { os << std::forward<T>(value) } -> std::same_as<std::ostream&>;
};

/// @brief represents a nullable single or multiple(vector) AbsoluteStyle
/// storage.
class StyleStack {
  public:
    // single by default
    StyleStack() = default;

    // Observe:

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

    auto is_multiple() const -> bool { return is_multiple_; }

    // Stack operations:

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

    // Transform:

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

// TODO:

template <typename T> struct StyledRef {};

template <typename T> inline constexpr auto styled(T&& value) -> StyledRef<T>;

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
    AbsoluteStyle base_style_ = absolute(Style());

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
    StyledOstream(std::ostream& os) : os_(os) {}

    template <detail::OstreamOutputable T>
    friend auto operator<<(StyledOstream& so, T&& value) -> StyledOstream& {
        using ValueType = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<ValueType, Style>
                      || std::is_same_v<ValueType, AbsoluteStyle>) {
            so.push_style(value);
            so.output_style(value);
        } else if constexpr (std::is_same_v<ValueType, style_reset_t>) {
            so.reset_style();
            so.output_style(so.current_style());
        } else {
            so.os_ << std::forward<T>(value);
        }
        return so;
    }

    friend auto operator<<(StyledOstream& so, style_pop_t) -> StyledOstream& {
        so.pop_style();
        if (so.style_enabled_) so.os_ << so.current_style();
        return so;
    }

  private:
    void output_style(Style style) {
        if (style_enabled_) os_ << style;
    }
    void output_style(AbsoluteStyle absolute) {
        if (style_enabled_) os_ << absolute;
    }

    std::ostream& os_;
};

} // namespace deco

#endif // !DECOTERM_STYLED_HPP
