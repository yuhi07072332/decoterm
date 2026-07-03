#ifndef DECOTERM_FORMATTER_HPP
#define DECOTERM_FORMATTER_HPP

#include "decoterm.hpp"

#include <format>

namespace std {

/// @brief std::formatter for Style and AbsoluteStyle.
template<deco::detail::OutputableStyle StyleT>
struct formatter<StyleT> {

    constexpr formatter() = default;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(StyleT style, std::format_context& ctx) const {
        using namespace deco::detail;

        g_style_output.push_style_if(style);
        if (!g_style_output.is_enabled) return ctx.out();
        return style.to_escape(ctx.out());
    }
};

/// @brief std::formatter for pop.
template <>
struct formatter<deco::stylepop_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(deco::stylepop_t, std::format_context& ctx) const {
        using namespace deco::detail;
        g_style_output.pop_style_if();
        auto current = g_style_output.current_style();
        if (!g_style_output.is_enabled) return ctx.out();
        return current.to_escape(ctx.out());
    }
};


};  // namespace std

#endif // !DECOTERM_FORMATTER_HPP
