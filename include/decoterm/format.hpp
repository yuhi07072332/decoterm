// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECO_FORMATTER_HPP
#define DECO_FORMATTER_HPP

#include "style.hpp"
#include "styled_out.hpp"

#include <format>
#include <iterator>
#include <version>

#if __cplusplus < 202302L
#include <type_traits>
#endif

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202403L
#define DECO_ENABLE_PRINT 1 // NOLINT
#include <print>
#else
#define DECO_ENABLE_PRINT 0
#endif

namespace deco {

namespace detail {
#if __cplusplus >= 202302L
template <typename Arg, typename CharT = char>
concept formattable = std::formattable<Arg, CharT>;
#else
// fallback for C++20. This only checks if `std::formatter<Arg>` exists
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
} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledFormat                       ║
// ╚═════════════════════════════════════════════════════════╝

class StyledFormat : StyleOutputState<StyledFormat> {
  public:
    template <std::output_iterator<const char&> OutputIt>
    auto vformat_to(OutputIt out, std::string_view fmt, std::format_args args) -> OutputIt {
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(std::format_string<Args...> fmt, Args&&... args)
        -> std::string {
        return std::vformat(fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(detail::outputable_style auto style,
                std::format_string<Args...> fmt,
                Args&&... args) -> std::string {
        std::string buf;
        auto out = std::back_inserter(buf);
        out = std::vformat_to(out, fmt.get(), std::make_format_args(style));
        out = std::vformat_to(out, fmt.get(), std::make_format_args(args...));
        return buf;
    }
private:
    friend class StyleOutputState<StyledFormat>;

    void output_style_impl(detail::outputable_style auto style) const {
    }
};

}; // namespace deco

// ╔═════════════════════════════════════════════════════════╗
// ║                       Formatters                        ║
// ╚═════════════════════════════════════════════════════════╝

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
        ctx.advance_to(styled.style().to_escape(ctx.out()));
        return formatter<deco::style_reset_t> {}.format(deco::reset, ctx);
    }
};

}; // namespace std

#endif // !DECO_FORMATTER_HPP
