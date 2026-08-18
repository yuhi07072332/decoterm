// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_FORMAT_HPP
#define DECOTERM_FORMAT_HPP

#include "output.hpp"
#include "style.hpp"

#include <format>
#include <iterator>
#include <optional>
#include <type_traits>
#include <variant>
#include <version>

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202403L
#define DECO_ENABLE_PRINT
#include <print>
#endif

namespace deco {

namespace concepts {

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

} // namespace concepts

namespace detail {

struct StyledFormatContext {
    constexpr StyledFormatContext(AbsoluteStyle current_style,
                                  bool style_enabled)
        : current_style(current_style),
          style_enabled(style_enabled) {}

    AbsoluteStyle current_style;
    bool style_enabled;
};

// StyledRef with style context.
// This is used instead of StyledRef in StyledFormat.
template <typename StyledRefT>
struct FStyledRef {
    constexpr FStyledRef(StyledRefT styled, StyledFormatContext context)
        : styled(styled),
          context(context) {}

    StyledRefT styled;
    StyledFormatContext context; // NOLINT
};

// Replace StyledRef<T>& with FStyledRef<T>.
template <typename Arg>
constexpr auto process_arg(detail::StyledFormatContext context, Arg&& arg)
    -> decltype(auto) {
    if constexpr (concepts::styled<Arg>) {
        return FStyledRef(arg, context);
    } else return std::forward<Arg>(arg); // NOLINT
}

template <typename Arg>
using processed_arg_t = std::conditional_t<concepts::styled<Arg>,
                                           FStyledRef<std::remove_cvref_t<Arg>>,
                                           Arg>;

// this is used with process_arg(), since make_format_args don't take rvalue
// reference.
template <typename... Args>
constexpr auto make_format_args(Args&&... args /*NOLINT*/) {
    return std::make_format_args(args...);
}

} // namespace detail

} // namespace deco

// ╔═════════════════════════════════════════════════════════╗
// ║                       Formatters                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

/// @brief formatter for Style types, e.g. `Style` and `AbsoluteStyle`
template <deco::concepts::style StyleT>
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
        return deco::abs(deco::null_style).to_escape(ctx.out());
    }
};

/// @brief formatter for styled values
template <deco::concepts::styled_ref StyledRefT>
    requires deco::concepts::formattable<typename StyledRefT::value_type>
struct formatter<StyledRefT> {
    std::formatter<typename StyledRefT::value_type, char> value_formatter;

    bool reset_on_end;

    constexpr formatter(bool reset_on_end = true)
        : reset_on_end(reset_on_end) {}

    constexpr auto parse(std::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    auto format(const StyledRefT& styled, /*NOLINT*/
                std::format_context& ctx) const {
        ctx.advance_to(styled.style().to_escape(ctx.out()));
        ctx.advance_to(value_formatter.format(styled.value(), ctx));
        if (reset_on_end)
            ctx.advance_to(
                formatter<deco::style_reset_t> {}.format(deco::reset, ctx));
        return ctx.out();
    }
};

// internal formatter
template <deco::concepts::styled_ref StyledRefT>
    requires deco::concepts::formattable<typename StyledRefT::value_type>
struct formatter<deco::detail::FStyledRef<StyledRefT>>
    : public formatter<StyledRefT> {
    using formatter<StyledRefT>::value_formatter;
    using formatter<StyledRefT>::parse;

    constexpr formatter() : formatter<StyledRefT>(false) {}

    auto format(const deco::detail::FStyledRef<StyledRefT>& fstyled, /*NOLINT*/
                std::format_context& ctx) const {
        if (fstyled.context.style_enabled)
            ctx.advance_to(fstyled.styled.style().to_escape(ctx.out()));
        ctx.advance_to(value_formatter.format(fstyled.styled.value(), ctx));
        if (fstyled.context.style_enabled)
            ctx.advance_to(fstyled.context.current_style.to_escape(ctx.out()));
        return ctx.out();
    }
};

}; // namespace std

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledFormat                       ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief Format string type used by StyledFormat::print().
template <typename... Args>
using FormatString = std::format_string<detail::processed_arg_t<Args>...>;

/// @brief A stateful writer similar to `StyledOstream`, but with
/// `std::formatter` support.
/// @details Format arguments cannot contain Style types, reset, or pop. Use
/// print(style, ...), styled(), push(), reset(), or pop() instead.
class StyledFormat : public StyleState, public StyleStateSetter<StyledFormat> {
    using stream_type = std::variant<std::FILE*, std::ostream*>;

  public:
    StyledFormat() = default;
    StyledFormat(StyleState state) : StyleState(std::move(state)) {}

    /* ----- Style operations ----- */

    /// Similar to `styled_os << style`, where `styled_os` is a StyledOstream
    /// object.
    auto push(concepts::style auto style) -> StyledFormat& {
        push_style(style);
        current_is_pending_ = true;
        return *this;
    }

    /// Similar to `styled_os << pop`, where `styled_os` is a StyledOstream
    /// object.
    auto pop() -> StyledFormat& {
        pop_style();
        current_is_pending_ = true;
        return *this;
    }

    /// Similar to `styled_os << reset`, where `styled_os` is a StyledOstream
    /// object.
    auto reset() -> StyledFormat& {
        reset_style();
        current_is_pending_ = true;
        return *this;
    }

    /* ----- format ----- */

    template <std::output_iterator<const char&> OutputIt,
              concepts::style StyleT,
              typename... Args>
    auto format_to(OutputIt out,
                   StyleT style,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt {
        (check_arg<Args>(), ...);
        out = ensure_context(out);
        const AbsoluteStyle current =
            detail::apply_style(current_style(), style);

        if (!style.is_null()) out = output_style(out, style);
        out = std::vformat_to(
            out,
            fmt.get(),
            detail::make_format_args(process_arg(
                detail::StyledFormatContext(current, style_enabled()),
                std::forward<Args>(args))...));
        if (!style.is_null()) out = output_style(out, current_style());
        return out;
    }

    template <std::output_iterator<const char&> OutputIt, typename... Args>
    auto format_to(OutputIt out,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt {
        return format_to(out, null_style, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(concepts::style auto style,
                std::format_string<Args...> fmt,
                Args&&... args) -> std::string {
        std::string buf;
        format_to(
            std::back_inserter(buf), style, fmt, std::forward<Args>(args)...);
        return buf;
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(std::format_string<Args...> fmt, Args&&... args)
        -> std::string {
        std::string buf;
        format_to(std::back_inserter(buf), fmt, std::forward<Args>(args)...);
        return buf;
    }

  private:
    template <typename Arg>
    static consteval void check_arg() {
        using arg_type = std::remove_cvref_t<Arg>;
        if constexpr (concepts::styled<arg_type>)
            detail::check_styled_ref<Arg>();
        else
            static_assert(
                (!concepts::style<arg_type>
                 && !std::is_same_v<arg_type, style_reset_t>
                 && !std::is_same_v<arg_type, style_pop_t>),
                "deco::StyledFormat: Style, reset, pop are disallowed as "
                "format arguments. Use print(style, ...), styled(), "
                "push(), reset(), or pop() instead.");
    }

    template <std::output_iterator<const char&> OutputIt,
              concepts::style StyleT>
    auto output_style(OutputIt out, StyleT style) const -> OutputIt {
        if (!style_enabled()) return out;
        return fallback_style(style).to_escape(out);
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

    // Whether the current style needs to be emitted before the next output.
    bool current_is_pending_ = false;

    // ──────────────────────── std::print extensions ────────────────────────

#ifdef DECO_ENABLE_PRINT
  public:
    /* ----- print ----- */
    auto set_stream(FILE* f) -> StyledFormat& {
        stream_.emplace<FILE*>(f);
        return *this;
    }

    auto set_stream(std::ostream& os) -> StyledFormat& {
        stream_.emplace<std::ostream*>(&os);
        return *this;
    }

    /// @brief Prints with a temporary style to the stream.
    template <concepts::style StyleT, typename... Args>
    auto print(StyleT style, FormatString<Args...> fmt, Args&&... args)
        -> StyledFormat& {
        (check_arg<Args>(), ...);
        ensure_context();
        const AbsoluteStyle current =
            detail::apply_style(current_style(), style);

        if (!style.is_null()) output_style(style);
        print_stream(
            fmt,
            process_arg(detail::StyledFormatContext(current, style_enabled()),
                        std::forward<Args>(args))...);
        if (!style.is_null()) output_style(current_style());
        return *this;
    }

    template <typename... Args>
    auto print(FormatString<Args...> fmt, Args&&... args) -> StyledFormat& {
        print(null_style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    // TODO: println
  private:
    template <typename... Args>
    void print_stream(std::format_string<Args...> fmt, Args&&... args) const {
        if (std::holds_alternative<FILE*>(stream_)) {
            std::print(
                std::get<FILE*>(stream_), fmt, std::forward<Args>(args)...);
        } else if (std::holds_alternative<std::ostream*>(stream_)) {
            std::print(*std::get<std::ostream*>(stream_),
                       fmt,
                       std::forward<Args>(args)...);
        }
    }

    void output_style(concepts::style auto style) const {
        if (!style_enabled()) return;
        print_stream("{}", fallback_style(style));
    }

    void ensure_context() {
        const auto update = update_context();
        const AbsoluteStyle current = current_style();
        if (update) output_style(*update);
        if (current_is_pending_ && (!update || current != *update))
            output_style(current);
        current_is_pending_ = false;
    }

    stream_type stream_ = stream_type(std::in_place_type<FILE*>, stdout);

#endif // DECO_ENABLE_PRINT
};

#ifdef DECO_ENABLE_PRINT

/// @brief Creates a StyledFormat with context tracking enabled.
/// @details equivalent to
/// `StyledFormat().enable_context_tracking().set_stream(f)`
inline auto styled_fmt(std::FILE* f = stdout) -> StyledFormat {
    return StyledFormat().enable_context_tracking().set_stream(f);
}

/// @brief Creates a StyledFormat with context tracking enabled.
/// @details equivalent to
/// `StyledFormat().enable_context_tracking().set_stream(os)`
inline auto styled_fmt(std::ostream& os) -> StyledFormat {
    return StyledFormat().enable_context_tracking().set_stream(os);
}

#endif // DECO_ENABLE_PRINT

}; // namespace deco

#ifdef DECO_ENABLE_PRINT
#undef DECO_ENABLE_PRINT
#endif

#endif // !DECOTERM_FORMAT_HPP
