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

namespace deco::detail {
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
} // namespace deco::detail

// ╔═════════════════════════════════════════════════════════╗
// ║                       Formatters                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

/// @brief formatter for Style types e.g. `Style`, `AbsoluteStyle`
template <deco::detail::style StyleT>
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
template <deco::detail::formattable T, deco::detail::style StyleT>
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

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledFormat                       ║
// ╚═════════════════════════════════════════════════════════╝

class StyledFormat : public StyleState, public StyleStateOption<StyledFormat> {
  public:
    template <std::output_iterator<const char&> OutputIt, detail::style StyleT>
    auto vformat_to(OutputIt out,
                    StyleT style,
                    std::string_view fmt,
                    std::format_args args) -> OutputIt {
        if (auto update = update_context()) out = update->style.to_escape(out);
        out = style.to_escape(out);
        return std::format_to(out, fmt, args);
    }

    template <std::output_iterator<const char&> OutputIt>
    auto vformat_to(OutputIt out, std::string_view fmt, std::format_args args)
        -> OutputIt {
        if (auto update = update_context()) out = update->style.to_escape(out);
        return std::format_to(out, fmt, args);
    }

    template <std::output_iterator<const char&> OutputIt,
              detail::style StyleT,
              typename... Args>
    auto format_to(OutputIt out,
                   StyleT style,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt { // NOLINT
        return vformat_to(
            out, style, fmt.get(), std::make_format_args(args...));
    }

    template <std::output_iterator<const char&> OutputIt, typename... Args>
    auto format_to(OutputIt out,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt { // NOLINT
        return vformat_to(out, fmt.get(), std::make_format_args(args...));
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(std::format_string<Args...> fmt, Args&&... args) // NOLINT
        -> std::string {
        std::string buf;
        return vformat_to(
            std::back_inserter(buf), fmt, std::make_format_args(args...));
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(detail::style auto style,
                std::format_string<Args...> fmt,
                Args&&... args) -> std::string { // NOLINT
        std::string buf;
        return vformat_to(
            std::back_inserter(buf), fmt, std::make_format_args(args...));
    }
};

#if DECO_ENABLE_PRINT

class StyledPrint : StyledFormat {
    template <detail::style StyleT, typename... Args>
    void print(std::FILE* f,
               StyleT style,
               std::format_string<Args...> fmt,
               Args&&... args) {
        print(std::print, f, style, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void print(std::FILE* f, std::format_string<Args...> fmt, Args&&... args) {
        print(std::print, f, fmt, std::forward<Args>(args)...);
    }

    template <detail::style StyleT, typename... Args>
    void print(StyleT style, std::format_string<Args...> fmt, Args&&... args) {
        print(std::print, stdout, style, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void print(std::format_string<Args...> fmt, Args&&... args) {
        print(std::print, stdout, fmt, std::forward<Args>(args)...);
    }

    template <detail::style StyleT, typename... Args>
    void print(std::ostream& os,
               StyleT style,
               std::format_string<Args...> fmt,
               Args&&... args) {
        print(std::print, os, style, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void
    print(std::ostream& os, std::format_string<Args...> fmt, Args&&... args) {
        print(std::print, os, fmt, std::forward<Args>(args)...);
    }

    //TODO: println

  private:
    /// @tparam Fn `std::print` or `std::println`
    /// @tparam Stream 'std::FILE*' or 'std::ostream'
    template <typename Fn,
              typename Stream,
              detail::style StyleT,
              typename... Args>
    void print(Fn&& printfn /*NOLINT*/,
               Stream& f,
               StyleT style,
               std::format_string<Args...> fmt,
               Args&&... args) {
        if (auto update = update_context())
            printfn(f, "{}", *update, std::forward<Args>(args)...);
        printfn(f, "{}", style);
        printfn(f, fmt, std::forward<Args>(args)...);
    }

    template <typename Fn, typename Stream, typename... Args>
    void print(Fn&& printfn /*NOLINT*/,
               Stream& f,
               std::format_string<Args...> fmt,
               Args&&... args) {
        print(printfn, f, default_style, fmt, std::forward<Args>(args)...);
    }
};

#endif // DECO_ENABLE_PRINT

}; // namespace deco

#endif // !DECO_FORMATTER_HPP
