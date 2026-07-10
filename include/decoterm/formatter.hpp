#ifndef DECOTERM_FORMATTER_HPP
#define DECOTERM_FORMATTER_HPP

#include "decoterm.hpp"

#include <format>

namespace std {

/// @brief std::formatter for Style and AbsoluteStyle.
template<deco::detail::OutputableStyle StyleT>
struct formatter<StyleT> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(StyleT style, std::format_context& ctx) const {
        using namespace deco::detail;

        output_control().push_style_if(style);
        if (!output_control().is_enabled) return ctx.out();
        return style.to_escape(ctx.out());
    }
};

/// @brief std::formatter for pop.
template <>
struct formatter<deco::stylepop_t> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::stylepop_t, std::format_context& ctx) const {
        using namespace deco::detail;
        output_control().pop_style_if();
        auto current = output_control().current_style();
        if (!output_control().is_enabled) return ctx.out();
        return current.to_escape(ctx.out());
    }
};


};  // namespace std

#endif // !DECOTERM_FORMATTER_HPP
