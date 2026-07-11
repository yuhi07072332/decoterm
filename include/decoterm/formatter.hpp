// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_FORMATTER_HPP
#define DECOTERM_FORMATTER_HPP

#include "decoterm.hpp"

#include <format>

namespace std {

/// @brief std::formatter for Style types.
template <deco::detail::OutputableStyle StyleT> struct formatter<StyleT> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(StyleT style, std::format_context& ctx) const {
        using namespace deco::detail;

        output_state().push_style(style);
        if (!output_state().style_enabled()) return ctx.out();
        return style.to_escape(ctx.out());
    }
};

/// @brief std::formatter for deco::pop.
template <> struct formatter<deco::style_pop_t> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::style_pop_t, std::format_context& ctx) const {
        using namespace deco::detail;
        output_state().pop_style();
        auto current = output_state().current_style();
        if (!output_state().style_enabled()) return ctx.out();
        return current.to_escape(ctx.out());
    }
};

/// @brief std::formatter for deco::reset.
template <> struct formatter<deco::style_reset_t> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::style_reset_t, std::format_context& ctx) const {
        using namespace deco::detail;
        output_state().reset_style_stack();
        if (!output_state().style_enabled()) return ctx.out();
        return deco::absolute().to_escape(ctx.out());
    }
};

}; // namespace std

#endif // !DECOTERM_FORMATTER_HPP
