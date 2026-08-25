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
    AbsoluteStyle current_style = deco::abs(deco::null_style);
    bool style_enabled = true;
};

// `Styled` with format context.
// This is used instead of `Styled` in `StyledFormat`.
template <concepts::style StyleT, typename... Ts>
struct FStyled {
    using StyledT = Styled<StyleT, Ts...>;
    constexpr FStyled(StyledT styled, StyledFormatContext context)
        : styled(std::move(styled)),
          context(context) {}

    StyledT styled;
    StyledFormatContext context; // NOLINT
};

// Replace StyledRef<T>& with FStyledRef<T>.
template <typename Arg>
constexpr auto process_arg(detail::StyledFormatContext context, Arg&& arg)
    -> decltype(auto) {
    if constexpr (detail::styled<Arg>) {
        return FStyled(arg, context);
    } else return std::forward<Arg>(arg); // NOLINT
}

template <typename Arg>
using processed_arg_t = decltype(process_arg(
    std::declval<detail::StyledFormatContext>(), std::declval<Arg>()));

// this is used with process_arg(), since make_format_args don't take rvalue
// reference.
template <typename... Args>
constexpr auto make_format_args(Args&&... args /*NOLINT*/) {
    return std::make_format_args(args...);
}

template <typename Arg>
static consteval void check_arg() {
    using arg_type = std::remove_cvref_t<Arg>;
    static_assert((!concepts::style<arg_type>
                   && !std::is_same_v<arg_type, style_reset_t>
                   && !std::is_same_v<arg_type, style_pop_t>),
                  "deco::StyledFormat: Style, reset, pop are disallowed as "
                  "format arguments. Use print(style, ...), styled(), "
                  "push(), reset(), or pop() instead.");
}

} // namespace detail

} // namespace deco

// ╔═════════════════════════════════════════════════════════╗
// ║                       Formatters                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

/// @brief formatter for style types
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

/// @brief formatter for **single-value** `Styled`
template <deco::concepts::style StyleT, typename T>
    requires(!deco::detail::styled<T>
             && deco::concepts::formattable<
                 typename deco::detail::unwrap_constref<T>::type>)
struct formatter<deco::Styled<StyleT, T>> {
    // 这个是实现`FStyled`的`formatter`能设置context的同时，能复用(也就是继承)此`formatter`
    // 的`parse()`逻辑的HACK。这样就不用为了`FStyled`重复写相同的逻辑了。
    mutable deco::detail::StyledFormatContext context;

    std::formatter<
        std::remove_cvref_t<typename deco::detail::unwrap_constref<T>::type>,
        char>
        value_formatter;

    constexpr formatter() = default;
    constexpr formatter(deco::detail::StyledFormatContext context)
        : context(context) {}

    constexpr auto parse(std::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    auto format(const deco::Styled<StyleT, T>& styled, /*NOLINT*/
                std::format_context& ctx) const {
        using namespace deco;
        detail::apply_styled(
            context.current_style,
            [&ctx, this](const T& value) {
                ctx.advance_to(value_formatter.format(value, ctx));
            },
            [&ctx, this]<concepts::style StyleType>(StyleType style) {
                if (context.style_enabled)
                    ctx.advance_to(formatter<StyleType> {}.format(style, ctx));
            },
            styled);
        return ctx.out();
    }
};

/// @brief formatter for **multi-value/nested** `Styled`
/// @details This cannot specify format specs for each args.
template <deco::concepts::style StyleT, typename... Ts>
    requires((deco::detail::styled<Ts>
              || deco::concepts::formattable<
                  typename deco::detail::unwrap_constref<Ts>::type>)
             && ...)
struct formatter<deco::Styled<StyleT, Ts...>> {
    // see single-value version formatter
    mutable deco::detail::StyledFormatContext context;

    constexpr formatter() = default;
    constexpr formatter(deco::detail::StyledFormatContext context)
        : context(context) {}

    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const deco::Styled<StyleT, Ts...>& styled, /*NOLINT*/
                std::format_context& ctx) const {
        using namespace deco;
        detail::apply_styled(
            context.current_style,
            [&ctx, this]<typename P>(const P& value) {
                using value_type = std::remove_cvref_t<
                    typename deco::detail::unwrap_constref<P>::type>;

                if constexpr (detail::styled<P>) {
                    std::formatter<value_type, char> styled_formatter {context};
                    ctx.advance_to(styled_formatter.format(value, ctx));
                } else {
                    std::formatter<value_type, char> value_formatter {};
                    ctx.advance_to(value_formatter.format(value, ctx));
                }
            },
            [&ctx, this]<concepts::style StyleType>(StyleType style) {
                if (context.style_enabled)
                    ctx.advance_to(formatter<StyleType> {}.format(style, ctx));
            },
            styled);
        return ctx.out();
    }
};

template <deco::concepts::style StyleT, typename... Ts>
    requires((deco::detail::styled<Ts>
              || deco::concepts::formattable<
                  typename deco::detail::unwrap_constref<Ts>::type>)
             && ...)
struct formatter<deco::detail::FStyled<StyleT, Ts...>>

    : public formatter<deco::Styled<StyleT, Ts...>> {
    using underlying = formatter<deco::Styled<StyleT, Ts...>>;

    using underlying::context;

    constexpr formatter() = default;

    auto format(const deco::detail::FStyled<StyleT, Ts...>& fstyled, /*NOLINT*/
                std::format_context& ctx) const {
        context = fstyled.context;
        return underlying::format(fstyled.styled, ctx);
    }
};

} // namespace std

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

  public:
    StyledFormat() = default;
    StyledFormat(StyleState state) : StyleState(std::move(state)) {}

    /* ----- Style operations ----- */

    auto push(concepts::style auto style) -> StyledFormat& {
        this->push_style(style);
        current_is_pending_ = true;
        return *this;
    }

    auto pop() -> StyledFormat& {
        this->pop_style();
        current_is_pending_ = true;
        return *this;
    }

    auto reset() -> StyledFormat& {
        this->reset_style();
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
        (detail::check_arg<Args>(), ...);
        out = this->ensure_context(out);
        const AbsoluteStyle current =
            detail::apply_style(this->current_style(), style);

        if (!style.is_null()) out = this->output_style(out, style);
        out = std::vformat_to(
            out,
            fmt.get(),
            detail::make_format_args(process_arg(
                detail::StyledFormatContext(current, this->style_enabled()),
                std::forward<Args>(args))...));
        if (!style.is_null())
            out = this->output_style(out, this->current_style());
        return out;
    }

    template <std::output_iterator<const char&> OutputIt, typename... Args>
    auto format_to(OutputIt out,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> OutputIt {
        return this->format_to(
            out, null_style, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(concepts::style auto style,
                std::format_string<Args...> fmt,
                Args&&... args) -> std::string {
        std::string buf;
        this->format_to(
            std::back_inserter(buf), style, fmt, std::forward<Args>(args)...);
        return buf;
    }

    template <typename... Args>
    [[nodiscard]]
    auto format(std::format_string<Args...> fmt, Args&&... args)
        -> std::string {
        std::string buf;
        this->format_to(
            std::back_inserter(buf), fmt, std::forward<Args>(args)...);
        return buf;
    }

  private:
    template <std::output_iterator<const char&> OutputIt,
              concepts::style StyleT>
    auto output_style(OutputIt out, StyleT style) const -> OutputIt {
        if (!this->style_enabled()) return out;
        return this->fallback_style(style).to_escape(out);
    }

    template <std::output_iterator<const char&> OutputIt>
    auto ensure_context(OutputIt out) -> OutputIt {
        const auto update = this->update_context();
        const AbsoluteStyle current = this->current_style();

        if (update) out = this->output_style(out, *update);
        if (current_is_pending_ && (!update || current != *update))
            out = this->output_style(out, current);
        current_is_pending_ = false;
        return out;
    }

    // Whether the current style needs to be emitted before the next output.
    bool current_is_pending_ = false;
};

// ───────────────────────── <print> extensions ──────────────────────

#ifdef DECO_ENABLE_PRINT

class StyledPrint : public StyleState, public StyleStateSetter<StyledPrint> {
    using stream_type = std::variant<std::FILE*, std::ostream*>;

  public:
    StyledPrint() = default;
    StyledPrint(StyleState state) : StyleState(std::move(state)) {}

    auto set_stream(FILE* f) -> StyledPrint& {
        stream_.emplace<FILE*>(f);
        return *this;
    }

    auto set_stream(std::ostream& os) -> StyledPrint& {
        stream_.emplace<std::ostream*>(&os);
        return *this;
    }

    /// @brief Prints with a temporary style to the stream.
    template <concepts::style StyleT, typename... Args>
    auto print(StyleT style, FormatString<Args...> fmt, Args&&... args)
        -> StyledPrint& {
        (detail::check_arg<Args>(), ...);
        this->ensure_context();
        const AbsoluteStyle current =
            detail::apply_style(this->current_style(), style);

        if (!style.is_null()) this->output_style(style);
        this->print_stream(fmt,
                           process_arg(detail::StyledFormatContext(
                                           current, this->style_enabled()),
                                       std::forward<Args>(args))...);
        if (!style.is_null()) this->output_style(this->current_style());
        return *this;
    }

    template <typename... Args>
    auto print(FormatString<Args...> fmt, Args&&... args) -> StyledPrint& {
        this->print(null_style, fmt, std::forward<Args>(args)...);
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
        if (!this->style_enabled()) return;
        this->print_stream("{}", this->fallback_style(style));
    }

    void ensure_context() {
        const auto update = this->update_context();
        const AbsoluteStyle current = this->current_style();
        if (update) this->output_style(*update);
        if (current_is_pending_ && (!update || current != *update))
            this->output_style(current);
        current_is_pending_ = false;
    }

    stream_type stream_ = stream_type(std::in_place_type<FILE*>, stdout);
    // Whether the current style needs to be emitted before the next output.
    bool current_is_pending_ = false;
};

/// @brief Creates a StyledFormat with context tracking enabled.
/// @details equivalent to
/// `StyledFormat().enable_context_tracking().set_stream(f)`
inline auto styled_print(std::FILE* f = stdout) -> StyledPrint {
    return StyledPrint().enable_context_tracking().set_stream(f);
}

/// @brief Creates a StyledFormat with context tracking enabled.
/// @details equivalent to
/// `StyledFormat().enable_context_tracking().set_stream(os)`
inline auto styled_print(std::ostream& os) -> StyledPrint {
    return StyledPrint().enable_context_tracking().set_stream(os);
}

#endif // DECO_ENABLE_PRINT

} // namespace deco

#ifdef DECO_ENABLE_PRINT
#undef DECO_ENABLE_PRINT
#endif

#endif // !DECOTERM_FORMAT_HPP
