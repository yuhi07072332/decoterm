// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_STYLEOUT_HPP
#define DECOTERM_STYLEOUT_HPP

#include "style.hpp"

#include <ostream>
#include <vector>
#include <variant>

namespace deco {

namespace detail {

template <typename T>
concept OstreamOutputable = requires(std::ostream& os, T&& value) {
    operator<<(os, std::forward<T>(value));
};

}

inline constexpr struct style_pop_t {} pop;

/// @brief A class representing Style output state.
class StyleOutputState {
public:
    StyleOutputState() = default;

    // ----- options -----

    auto base_style(Style base) -> StyleOutputState& {
        base_style_ = absolute(base);
        return *this;
    }

    auto enable_style_nesting(bool enable) -> StyleOutputState& { 
        if (nesting_enabled_ && !enable) {
            style_state_.emplace<SingleState>(nested().back());
        } else if (!nesting_enabled_ && enable) {
            if (base_style_.style != single().style)
                style_state_.emplace<NestedState>({ single() });
            else style_state_.emplace<NestedState>();
        }
        nesting_enabled_ = enable;
        return *this;
    }

    auto enable_style_output(bool enable) -> StyleOutputState& {
        output_enabled_ = enable;
        return *this;
    }

    auto enable_color_fallback(bool enable) -> StyleOutputState& {
        color_fallback_enabled_ = enable;
        return *this;
    }

    // ----- observe -----

    [[nodiscard]]
    auto base_style() const -> Style { return base_style_.style; }
    
    [[nodiscard]]
    auto style_nesting_enabled() const -> bool { return nesting_enabled_; }

    [[nodiscard]]
    auto style_output_enabled() const -> bool { return output_enabled_; }

    [[nodiscard]]
    auto color_fallback_enabled() const -> bool { return color_fallback_enabled_; }

    [[nodiscard]]
    auto current_style() const -> Style {
        if (nesting_enabled_) {
            const auto& nested_state = nested();
            return nested_state.empty() ? base_style_.style : nested_state.back().style;
        } else 
            return single().style;
    }

protected:
    using NestedState = std::vector<AbsoluteStyle>;
    using SingleState = AbsoluteStyle;

    auto single() -> SingleState& {
        return std::get<SingleState>(style_state_);
    }

    auto single() const -> const SingleState& {
        return std::get<SingleState>(style_state_);
    }

    auto nested() -> NestedState& {
        return std::get<NestedState>(style_state_);
    }

    auto nested() const -> const NestedState& {
        return std::get<NestedState>(style_state_);
    }

    void push_style(AbsoluteStyle style) {
        if (nesting_enabled_) {
            nested().push_back(style);
        } else {
            single() = style;
        }
    }

    void push_style(Style style) {
        push_style(absolute(current_style() | style));
    }

    void pop_style() {
        if (nesting_enabled_) {
            auto& nested_state = nested();
            if (!nested_state.empty()) nested_state.pop_back();
        } else {
            single() = base_style_;
        }
    }

    void reset_style() {
        if (nesting_enabled_) {
            nested().clear();
        } else single() = base_style_;
    }

    std::variant<SingleState, NestedState> style_state_ = NestedState();
    AbsoluteStyle base_style_ = absolute(Style());
    bool output_enabled_ = true;
    bool nesting_enabled_ = true;
    bool color_fallback_enabled_ = false;
};

/// @brief A stateful writer over an existing std::ostream.
/// @warning passed in std::ostream object must be valid in this life time.
class StyledOstream : public StyleOutputState{
public:
    StyledOstream(std::ostream& os) : os_(os) {}

    template <detail::OstreamOutputable T>
    friend auto operator<<(StyledOstream& sout, T&& value) -> StyledOstream& {
        if constexpr (std::is_same_v<T, Style> || std::is_same_v<T, AbsoluteStyle>) {
            if (sout.output_enabled_) sout << value;
            sout.push_style(value);
        } else if constexpr (std::is_same_v<T, style_reset_t>) {
            sout.reset_style();
        } else {
            sout.os_ << std::forward<T>(value);
        }
        return sout;
    }

    friend auto operator<<(StyledOstream& sout, style_pop_t) -> StyledOstream& {
        sout.pop_style();
        return sout;
    }

private:
    std::ostream& os_;
};

} // namespace deco

#endif // !DECOTERM_STYLEOUT_HPP
