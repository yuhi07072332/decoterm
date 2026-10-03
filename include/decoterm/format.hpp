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
#include <ostream>
#include <type_traits>
#include <version>

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L
#define DECO_ENABLE_STD_PRINT
#include <cstdio>
#include <print>
#endif

namespace deco {

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

namespace detail {

inline constexpr std::size_t FMT_BUFFER_SIZE = 500;

template <style_type StyleT>
struct FStyle {
    constexpr FStyle(StyleT style, const OutputConfig& cfg)
        : style(style),
          cfg(&cfg) {}

    StyleT style;
    NotNull<const OutputConfig*> cfg;
};

template <style_type StyleT, typename T>
struct FStyled {
    constexpr FStyled(const Styled<StyleT, T>& styled,
                      AbsoluteStyle current_style,
                      const OutputConfig* cfg)
        : styled(&styled),
          reset_style(current_style),
          cfg(cfg) {}

    NotNull<const Styled<StyleT, T>*> styled;
    AbsoluteStyle reset_style;
    const OutputConfig* cfg;
};

// Replace Styled<T> with FStyled<T>
template <typename Arg>
constexpr auto process_fmt_arg(const Arg& arg,
                               AbsoluteStyle current_style,
                               const OutputConfig* cfg) -> decltype(auto) {
    if constexpr (detail::styled<Arg>) return FStyled(arg, current_style, cfg);
    else return arg;
}

// Replace Styled<T> and style types
template <typename Arg>
constexpr auto process_fmt_arg_ctx(
    const Arg& arg,
    StyleContext& ctx,
    const OutputConfig& cfg,
    std::optional<AbsoluteStyle> current_style_override) -> decltype(auto) {
    if constexpr (detail::style_type<Arg>) {
        ctx.set_current_style(arg);
        return FStyle(arg, cfg);
    } else {
        return process_fmt_arg(
            arg, current_style_override.value_or(ctx.current_abstyle()), &cfg);
    }
}

template <typename Arg>
using process_fmt_arg_t =
    decltype(process_fmt_arg(std::declval<Arg>(),
                             std::declval<AbsoluteStyle>(),
                             std::declval<const OutputConfig*>));

template <typename Arg>
using process_fmt_arg_ctx_t =
    decltype(process_fmt_arg_ctx(std::declval<Arg>(),
                                 std::declval<StyleContext&>,
                                 std::declval<const OutputConfig*>(),
                                 std::declval<std::optional<AbsoluteStyle>>));

template <std::output_iterator<const char&> OutputIt, style_type StyleT>
auto write_style_to(OutputIt out, StyleT style, const OutputConfig& cfg) {
    if (!cfg.style_enabled()) return out;
    detail::fallback_color(style, cfg.color_mode());
    return style.to_escape(out);
}

using FormatBuf = Buffer<char, FMT_BUFFER_SIZE>;

template <std::output_iterator<const char&> OutputIt,
          style_type StyleT,
          typename... Args>
inline auto format_to(OutputIt out,
                      StyleT style,
                      AbsoluteStyle restore_style,
                      std::format_string<Args...> fmt,
                      Args&&... args /* NOLINT */) -> OutputIt {
    out = style.to_escape(out);
    out = std::vformat_to(
        out,
        fmt,
        std::make_format_args(process_fmt_arg(args, style, nullptr)...));
    if (!style.is_null()) out = restore_style.to_escape(out);
    return out;
}

template <std::output_iterator<const char&> OutputIt,
          style_type StyleT,
          typename... Args>
inline auto format_to(OutputIt out,
                      StyleT style,
                      std::format_string<Args...> fmt,
                      Args&&... args /* NOLINT */) -> OutputIt {
    return format_to(out, style, null_style, fmt, args...);
}

template <style_type StyleT, typename... Args>
inline auto format(StyleT style,
                   std::format_string<Args...> fmt,
                   Args&&... args) -> std::string {
    auto buf = FormatBuf();
    format_to(BufInserter(buf), style, fmt, std::forward<Args>(args)...);
    return {buf.data(), buf.size()};
}

#ifdef DECO_ENABLE_STD_PRINT

template <typename Stream, style_type StyleT, typename... Args>
inline auto print(Stream& stream,
                  StyleT style,
                  std::format_string<Args...> fmt,
                  Args&&... args /* NOLINT */) {
    auto buffer = FormatBuf();
    format_to(BufInserter(buffer), style, fmt, args...);
    std::print("{}", buffer.data());
}

template <typename Stream, style_type StyleT, typename... Args>
inline auto println(Stream& stream,
                    StyleT style,
                    std::format_string<Args...> fmt,
                    Args&&... args /* NOLINT */) {
    auto buffer = FormatBuf();
    format_to(BufInserter(buffer), style, fmt, args...);
    std::println("{}", buffer.data());
}

#endif // DECO_ENABLE_STD_PRINT

} // namespace detail

} // namespace deco

// ╔═════════════════════════════════════════════════════════╗
// ║                       formatters                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

/// formatter for style types
template <deco::detail::style_type StyleT>
struct formatter<StyleT> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(StyleT style, std::format_context& ctx) const {
        return style.to_escape(ctx.out());
    }
};

/// formatter for `deco::reset`
template <>
struct formatter<deco::detail::Reset> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::detail::Reset, std::format_context& ctx) const {
        return deco::abs(deco::null_style).to_escape(ctx.out());
    }
};

/// formatter for `styled()`
template <deco::detail::style_type StyleT, typename T>
    requires deco::formattable<T>
struct formatter<deco::detail::Styled<StyleT, T>> {
    std::formatter<std::remove_cvref_t<T>, char> value_formatter;

    constexpr formatter() = default;

    constexpr auto parse(std::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    auto format(const deco::detail::Styled<StyleT, T>& styled, /*NOLINT*/
                std::format_context& ctx) const {
        using namespace deco;

        ctx.advance_to(styled.style().to_escape(ctx.out()));
        ctx.advance_to(value_formatter.format(styled.value(), ctx));
        ctx.advance_to(abs(null_style).to_escape(ctx.out()));

        return ctx.out();
    }
};

template <deco::detail::style_type StyleT>
struct formatter<deco::detail::FStyle<StyleT>> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(const deco::detail::FStyle<StyleT> fstyle,
                std::format_context& ctx) const {
        return deco::detail::write_style_to(
            ctx.out(), fstyle.style, fstyle.cfg);
    }
};

template <deco::detail::style_type StyleT, typename T>
    requires deco::formattable<T>
struct formatter<deco::detail::FStyled<StyleT, T>>
    : formatter<deco::detail::Styled<StyleT, T>> {
    using formatter<deco::detail::Styled<StyleT, T>>::value_formatter;

    constexpr formatter() = default;

    auto format(const deco::detail::FStyled<StyleT, T>& fstyled,
                std::format_context& ctx) const {
        using namespace deco;

        const auto& styled = *fstyled.styled;
        const OutputConfig* cfg = fstyled.cfg;

        ctx.advance_to(
            deco::detail::write_style_to(ctx.out(), styled.style(), cfg));
        ctx.advance_to(value_formatter.format(styled.value(), ctx));
        ctx.advance_to(
            deco::detail::write_style_to(ctx.out(), fstyled.reset_style, cfg));
    }
};

} // namespace std

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                         format                          ║
// ╚═════════════════════════════════════════════════════════╝

template <std::output_iterator<const char&> OutputIt, typename... Args>
inline auto format_to(OutputIt out,
                      std::format_string<Args...> fmt,
                      Args&&... args) -> OutputIt {
    return detail::format_to(out, null_style, fmt, std::forward<Args>(args)...);
}

template <std::output_iterator<const char&> OutputIt, typename... Args>
inline auto format_to(OutputIt out,
                      Style style,
                      std::format_string<Args...> fmt,
                      Args&&... args) -> OutputIt {
    return detail::format_to(out, style, fmt, std::forward<Args>(args)...);
}

template <std::output_iterator<const char&> OutputIt, typename... Args>
inline auto format_to(OutputIt out,
                      AbsoluteStyle abstyle,
                      std::format_string<Args...> fmt,
                      Args&&... args) -> OutputIt {
    return detail::format_to(out, abstyle, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] inline auto format(std::format_string<Args...> fmt,
                                 Args&&... args) -> std::string {
    return detail::format(null_style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] inline auto format(Style style,
                                 std::format_string<Args...> fmt,
                                 Args&&... args) -> std::string {
    return detail::format(style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] inline auto format(AbsoluteStyle abstyle,
                                 std::format_string<Args...> fmt,
                                 Args&&... args) -> std::string {
    return detail::format(abstyle, fmt, std::forward<Args>(args)...);
}

#ifdef DECO_ENABLE_STD_PRINT

// ╔═════════════════════════════════════════════════════════╗
// ║                          print                          ║
// ╚═════════════════════════════════════════════════════════╝

template <typename... Args>
inline void print(std::format_string<Args...> fmt, Args&&... args) {
    detail::print<FILE*>(stdout, null_style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(Style s, std::format_string<Args...> fmt, Args&&... args) {
    detail::print<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(FILE* f, std::format_string<Args...> fmt, Args&&... args) {
    detail::print<FILE*>(f, null_style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(FILE* f,
                  Style s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(FILE* f,
                  AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::ostream& os,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print(os, null_style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::ostream& os,
                  Style s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::ostream& os,
                  AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::format_string<Args...> fmt, Args&&... args) {
    detail::println<FILE*>(
        stdout, null_style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(Style s, std::format_string<Args...> fmt, Args&&... args) {
    detail::println<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(FILE* f, std::format_string<Args...> fmt, Args&&... args) {
    detail::println<FILE*>(f, null_style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(FILE* f,
                    Style s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(FILE* f,
                    AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::ostream& os,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println(os, null_style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::ostream& os,
                    Style s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::ostream& os,
                    AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println(os, s, fmt, std::forward<Args>(args)...);
}

class Print {};

#endif // DECO_ENABLE_STD_PRINT

#if 0 // NOLINT

/// @brief Format string type used by StyledFormat::println().
template <typename... Args>
using FormatString = std::format_string<detail::processed_arg_t<Args>...>;

/// @brief A stateful writer similar to `StyledOstream`, but with
/// `std::formatter` support.
/// @details Format arguments cannot contain Style types, reset, or pop. Use
/// println(style, ...), styled(), push(), reset(), or pop() instead.
class StyledFormat : public StyleState, public StyleStateSetter<StyledFormat> {

  public:
    StyledFormat() = default;
    StyledFormat(StyleState state) : StyleState(std::move(state)) {}

    /* ----- Style operations ----- */

    auto push(detail::style auto style) -> StyledFormat& {
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
              detail::style StyleT,
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
    auto format(detail::style auto style,
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
              detail::style StyleT>
    auto output_style(OutputIt out, StyleT style) const -> OutputIt {
        if (!this->style_enabled()) return out;
        return this->fallback_style(style).to_escape(out);
    }

    template <std::output_iterator<const char&> OutputIt>
    auto ensure_context(OutputIt out) -> OutputIt {
        const auto update = this->update_context();
        const AbsoluteStyle current = abs(this->current_style());

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
    template <detail::style StyleT, typename... Args>
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

    void output_style(detail::style auto style) const {
        if (!this->style_enabled()) return;
        this->print_stream("{}", this->fallback_style(style));
    }

    void ensure_context() {
        const auto update = this->update_context();
        const AbsoluteStyle current = abs(this->current_style());
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

#endif

} // namespace deco

#ifdef DECO_ENABLE_PRINT
#undef DECO_ENABLE_PRINT
#endif

#endif // !DECOTERM_FORMAT_HPP
