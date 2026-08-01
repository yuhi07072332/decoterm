// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECO_FORMAT_HPP
#define DECO_FORMAT_HPP

#include "style.hpp"
#include "styled_out.hpp"

#include <format>
#include <iterator>
#include <optional>
#include <type_traits>
#include <variant>
#include <version>

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202403L
#define DECO_ENABLE_PRINT 1 //NOLINT
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

template <typename... Args>
consteval void check_sfmt_args() {
    static_assert(
        ((!detail::style<std::remove_cvref_t<Args>>
          && !std::is_same_v<std::remove_cvref_t<Args>, style_reset_t>)
         && ...),
        "StyledFormat: format args cannot contain Style types. Use push(), "
        "pop(), print(style...) instead.");
}

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
        return deco::absolute(deco::blank_style).to_escape(ctx.out());
    }
};

/// @brief formatter for `Styled`
template <deco::detail::formattable T, deco::detail::style StyleT>
struct formatter<deco::Styled<T, StyleT>> {
    std::formatter<std::remove_cvref_t<T>, char> value_formatter;

    constexpr auto parse(std::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    auto format(const deco::Styled<T, StyleT>& styled,
                std::format_context& ctx) const {
        ctx.advance_to(styled.style().to_escape(ctx.out()));
        ctx.advance_to(value_formatter.format(styled.value(), ctx));
        return formatter<deco::style_reset_t> {}.format(deco::reset, ctx);
    }
};

}; // namespace std

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledFormat                       ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A stateful writter similar to `StyledOstream`, but with
/// `std::formatter` support.
/// @detail Format args cannot contain Style types.
class StyledFormat : public StyleState, public StyleStateOption<StyledFormat> {
    using stream_type = std::variant<std::FILE*, std::ostream*>;

  public:
    StyledFormat() : StyleStateOption(static_cast<StyleState&>(*this)) {}
    StyledFormat(StyleState state)
        : StyleState(std::move(state)),
          StyleStateOption(static_cast<StyleState&>(*this)) {}

    /* ----- Style operations ----- */

    auto push(detail::style auto style) -> StyledFormat& {
        push_style(style);
        current_is_pending_ = true;
        return *this;
    }

    auto pop() -> StyledFormat& {
        pop_style();
        current_is_pending_ = true;
        return *this;
    }

    auto reset() -> StyledFormat& {
        reset_style();
        current_is_pending_ = true;
        return *this;
    }

    /* ----- format ----- */

    template <std::output_iterator<const char&> OutputIt,
              detail::style StyleT,
              typename... Args>
    auto format_to(OutputIt out,
                   StyleT style,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt { //NOLINT
        detail::check_sfmt_args<Args...>();
        out = ensure_context(out);
        if (!style.empty()) out = output_style(out, style);
        out = std::vformat_to(out, fmt.get(), std::make_format_args(args...));
        if (!style.empty()) out = output_style(out, current_style());
        return out;
    }

    template <std::output_iterator<const char&> OutputIt, typename... Args>
    auto format_to(OutputIt out,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt { //NOLINT
        return format_to(out, blank_style, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(detail::style auto style,
                std::format_string<Args...> fmt,
                Args&&... args) -> std::string { //NOLINT
        std::string buf;
        format_to(
            std::back_inserter(buf), style, fmt, std::forward<Args>(args)...);
        return buf;
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(std::format_string<Args...> fmt, Args&&... args) //NOLINT
        -> std::string {
        std::string buf;
        format_to(std::back_inserter(buf), fmt, std::forward<Args>(args)...);
        return buf;
    }

#if DECO_ENABLE_PRINT

    /* ----- print ----- */

    auto set_stream(std::FILE* f) -> StyledFormat& {
        stream_.emplace<std::FILE*>(f);
        return *this;
    }

    auto set_stream(std::ostream& os) -> StyledFormat& {
        stream_.emplace<std::ostream*>(&os);
        return *this;
    }

    template <detail::style StyleT, typename... Args>
    auto print(StyleT style, std::format_string<Args...> fmt, Args&&... args)
        -> StyledFormat& {
        detail::check_sfmt_args<Args...>();
        ensure_context();
        if (!style.empty()) output_style(style);
        print_stream(fmt, std::forward<Args>(args)...);
        if (!style.empty()) output_style(current_style());
        return *this;
    }

    template <typename... Args>
    auto print(std::format_string<Args...> fmt, Args&&... args)
        -> StyledFormat& {
        print(blank_style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    // TODO: println

#endif // DECO_ENABLE_PRINT

  private:
    template <std::output_iterator<const char&> OutputIt, detail::style StyleT>
    auto output_style(OutputIt out, StyleT style) const -> OutputIt {
        if (style_enabled()) return style.to_escape(out);
        return out;
    }

    template <std::output_iterator<const char&> OutputIt>
    auto ensure_context(OutputIt out) -> OutputIt {
        const auto update = update_context();
        const AbsoluteStyle current = current_style();

        if (update) out = output_style(out, *update);
        if (current_is_pending_ && (!update || current != *update))
            out = output_style(out, current);
        current_is_pending_ = false;
        return out;
    }

#if DECO_ENABLE_PRINT

    template <typename... Args>
    void print_stream(std::format_string<Args...> fmt, Args&&... args) const {
        if (std::holds_alternative<std::FILE*>(stream_)) {
            std::print(std::get<std::FILE*>(stream_),
                       fmt,
                       std::forward<Args>(args)...);
        } else if (std::holds_alternative<std::ostream*>(stream_)) {
            std::print(*std::get<std::ostream*>(stream_),
                       fmt,
                       std::forward<Args>(args)...);
        }
    }

    void output_style(detail::style auto style) const {
        if (!style_enabled()) return;
        print_stream("{}", style);
    }

    void ensure_context() {
        const auto update = update_context();
        const AbsoluteStyle current = current_style();
        if (update) output_style(*update);
        if (current_is_pending_ && (!update || current != *update))
            output_style(current);
        current_is_pending_ = false;
    }

    stream_type stream_ = stream_type(std::in_place_type<std::FILE*>, stdout);

#endif // DECO_ENABLE_PRINT

    // needs to output current style
    bool current_is_pending_ = false;
};

}; // namespace deco

#undef DECO_ENABLE_PRINT

#endif // !DECO_FORMAT_HPP
