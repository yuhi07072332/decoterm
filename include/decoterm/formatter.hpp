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
#include <type_traits>

namespace deco::detail {

#if __cplusplus >= 202302L

template <typename Arg, typename CharT = char>
concept formattable = std::formattable<Arg, CharT>;

#else

// Fallback for C++20. This only checks if `std::formatter<Arg>` exists
// and has `parse()` and `format()`.
template <typename Arg, typename CharT = char>
concept formattable =
    requires(std::formatter<std::remove_cvref_t<Arg>, CharT> formatter,
             Arg&& arg,
             std::basic_format_parse_context<CharT> parse_ctx,
             std::basic_format_context<char*, CharT> fmt_ctx) {
        {
            formatter.parse(parse_ctx)
        } -> std::same_as<typename decltype(parse_ctx)::iterator>;
        {
            formatter.format(std::forward<Arg>(arg), fmt_ctx)
        } -> std::same_as<typename decltype(fmt_ctx)::iterator>;
    };

#endif

} // namespace deco::detail

namespace std {

/// @brief formatter for Style types e.g. `Style`, `AbsoluteStyle`
template <deco::detail::outputable_style StyleT>
struct formatter<StyleT> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(StyleT style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

/// @brief formatter for `deco::reset`
template <>
struct formatter<deco::style_reset_t> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::style_reset_t, std::format_context& ctx) const {
        return deco::absolute(deco::default_style).to_escape(ctx.out());
    }
};

/// @brief formatter for `Styled`
template <deco::detail::formattable T, deco::detail::outputable_style StyleT>
struct formatter<deco::Styled<T, StyleT>> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(const deco::Styled<T, StyleT> styled,
                std::format_context& ctx) const {
        auto out = ctx.out();
        out = styled.style().to_escape(out);
        out = std::format_to(out, "{}", deco::reset);
    }
};

}; // namespace std

#endif // !DECOTERM_FORMATTER_HPP
