#ifndef DECOTERM_FORMATTER_HPP
#define DECOTERM_FORMATTER_HPP

#include "decoterm.hpp"

#include <format>

#ifdef DECOTERM_DETAIL_GLOBAL_TERMINAL
#include "terminal.hpp"
#endif // DECOTERM_DETAIL_GLOBAL_TERMINAL

// ╔═════════════════════════════════════════════════════════╗
// ║                        formatter                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

#ifdef DECOTERM_DETAIL_GLOBAL_TERMINAL

template<deco::detail::OutputableStyle StyleType>
struct formatter<StyleType> {

    constexpr formatter() = default;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(StyleType style, std::format_context& ctx) const {
        return deco::terminal.style_push_and_apply(ctx.out(), style);
    }
};

template<>
struct formatter<deco::StylePop> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(deco::StylePop, std::format_context &ctx) const {
        return deco::terminal.style_pop_and_apply(ctx.out());
    }
};

#else

template<deco::detail::OutputableStyle StyleType>
struct formatter<StyleType> {

    constexpr formatter() = default;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(StyleType style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

#endif // DECOTERM_DETAIL_GLOBAL_TERMINAL



};  // namespace std

#endif // !DECOTERM_FORMATTER_HPP
