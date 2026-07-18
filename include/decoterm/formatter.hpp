// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_FORMATTER_HPP
#define DECOTERM_FORMATTER_HPP

#include "style.hpp"
#include "styled.hpp"

#include <format>

namespace deco::detail {

// TODO: formatterable

// dude...
template <typename T>
concept formatterable = std::true_type::value;

} // namespace deco::detail

namespace std {

/// @brief std::formatter for Style types e.g. Style, AbsoluteStyle
template <deco::detail::outputable_style StyleType>
struct formatter<StyleType> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(StyleType style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

/// @brief std::formatter for deco::reset
template <>
struct formatter<deco::style_reset_t> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::style_reset_t, std::format_context& ctx) const {
        return deco::absolute(deco::default_style).to_escape(ctx.out());
    }
};

/// @brief std::formatter for StyledStorage
template <deco::detail::ostream_outputable T,
          deco::detail::outputable_style StyleType>
struct formatter<deco::StyledRef<deco::detail::StorageRef<T>, StyleType>> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto
    format(const deco::StyledRef<deco::detail::StorageRef<T>, StyleType> styled,
           std::format_context& ctx) const {
        auto out = ctx.out();
        out = styled.style().to_escape(out);
    }
};

}; // namespace std

#endif // !DECOTERM_FORMATTER_HPP
