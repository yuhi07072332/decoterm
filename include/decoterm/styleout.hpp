// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_STYLEOUT_HPP
#define DECOTERM_STYLEOUT_HPP

#include "style.hpp"

#include <iterator>
#include <ostream>
#include <vector>
#include <variant>

namespace deco {

namespace detail {

}

/// @brief A class representing Style output state.
class StyleOutputState {
public:
    StyleOutputState() : style_state_(NestedState({ absolute(default_style) })) {}

    // ----- options -----

    void set_style_output(bool enable) { output_enabled_ = enable; }

    void set_style_nesting(bool enable) { 
        if (nesting_enabled_ && !enable) {
            style_state_.emplace<SingleState>(std::get<NestedState>(style_state_).back());
        } else if (!nesting_enabled_ && enable) {
            style_state_.emplace<NestedState> ({ std::get<SingleState>(style_state_) });
        }
        nesting_enabled_ = enable;
    }

    void set_color_fallback(bool enable) { color_fallback_enabled_ = enable; }

    // ----- observe -----

    auto current_style() const -> Style {
        if (nesting_enabled_) {
            return nested().back().style;
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
            if (nested_state.size() > 1) nested_state.pop_back();
        } else {
            single() = absolute(default_style);
        }
    }

    std::variant<AbsoluteStyle, NestedState> style_state_;
    bool output_enabled_ = true;
    bool nesting_enabled_ = true;
    bool color_fallback_enabled_ = false;
};

/// @brief A lightweight stateful writer over an existing std::ostream.
class StyledOut : StyleOutputState{
public:
    StyledOut(std::ostream& os) : os_(os) {}

    template <typename T>
    friend auto operator<<(StyledOut& sout, T&& value) -> StyledOut& {
        if constexpr (std::is_same_v<T, Style> || std::is_same_v<T, AbsoluteStyle>) {
            if (sout.output_enabled_) value.to_escape(std::ostreambuf_iterator(sout.os_));
            sout.push_style(value);
        } else {
            sout.os_ << std::forward<T>(value);
        }
        return sout;
    }

private:
    std::ostream& os_;
};



} // namespace deco

#endif // !DECOTERM_STYLEOUT_HPP
