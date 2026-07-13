// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_FORMATTER_HPP
#define DECOTERM_FORMATTER_HPP

#include "style.hpp"

#include <format>

namespace std {

/// @brief std::formatter for deco::Style
template <> struct formatter<deco::Style> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::Style style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

/// @brief std::formatter for deco::AbsoluteStyle
template <> struct formatter<deco::AbsoluteStyle> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::AbsoluteStyle style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

/// @brief std::formatter for deco::reset
template <> struct formatter<deco::style_reset_t> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::style_reset_t, std::format_context& ctx) const {
        return deco::absolute(deco::Style()).to_escape(ctx.out());
    }
};

}; // namespace std

#endif // !DECOTERM_FORMATTER_HPP
