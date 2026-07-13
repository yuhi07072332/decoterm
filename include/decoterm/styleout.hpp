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

}

/// @brief A class representing Style output state.
class StyleOutState {
public:

    StyleOutState() = default;

    // ----- options -----

    void set_style_output(bool enable) { style_output_enabled_ = enable; }

    void set_style_stack(bool enable) { stack_enabled_ = enable; }

    void set_color_fallback(bool enable) { color_fallback_enabled_ = enable; }

    // ----- observe -----

    auto current_style() const -> AbsoluteStyle {

    }

private:
    std::variant<AbsoluteStyle, std::vector<AbsoluteStyle>> stack_;
    bool style_output_enabled_ = true;
    bool stack_enabled_ = true;
    bool color_fallback_enabled_ = false;
};

/// @brief A lightweight stateful writer over an existing std::ostream.
class StyleOut : StyleOutState{
public:
    StyleOut(std::ostream& os) : os_(os) {}

    template <typename T>
    friend auto operator<< (StyleOut& sout, T&& value) -> StyleOut& {
        sout.os_ << value;
        return sout;
    }

private:
    std::ostream& os_;
};

} // namespace deco

#endif // !DECOTERM_STYLEOUT_HPP
