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
#include <ostream>
#include <string_view>
#include <type_traits>
#include <variant>
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

template <typename T>
struct formatter_type {
    using type = std::formatter<T>;
};

template <typename Char, std::size_t N>
    requires requires { std::formatter<std::basic_string_view<Char>, Char>(); }
struct formatter_type<Char[N]> {
    using type = std::formatter<std::basic_string_view<Char>, Char>;
};

template <typename T>
using formatter_t = formatter_type<std::remove_cvref_t<T>>::type;

// inspired by {fmt} library
inline constexpr std::size_t FMT_INLINE_BUFFER_SIZE = 500;

using FormatBuf = Buffer<char, FMT_INLINE_BUFFER_SIZE>;

template <typename T, std::size_t N>
struct BufAppender {
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    explicit constexpr BufAppender(Buffer<T, N>& buf) : buf(&buf) {}

    constexpr auto operator*() -> BufAppender& { return *this; }
    constexpr auto operator++() -> BufAppender& { return *this; }
    constexpr auto operator++(int) -> BufAppender& { return *this; }

    constexpr auto operator=(T v) -> BufAppender& {
        buf->push_back(v);
        return *this;
    }

    NotNull<Buffer<T, N>*> buf;
};

template <style_type StyleT, typename T>
struct FStyled {
    constexpr FStyled(Styled<StyleT, T> styled, AbsoluteStyle current_style)
        : styled(std::move(styled)),
          restore_style(current_style) {}

    Styled<StyleT, T> styled;
    AbsoluteStyle restore_style;
};

// Replace `Styled` with `FStyled`
template <typename Arg>
constexpr auto process_fmt_arg(const Arg& arg, AbsoluteStyle current_style)
    -> decltype(auto) {
    if constexpr (detail::styled<Arg>)
        return FStyled(std::move(arg), current_style);
    else return arg;
}

template <typename... Args>
inline auto make_format_args(Args&&... args /* NOLINT */) {
    return std::make_format_args(args...);
}

template <std::output_iterator<const char&> OutputIt,
          style_type StyleT,
          typename... Args>
inline auto format_to_impl(OutputIt out,
                           StyleT style,
                           std::format_string<Args...> fmt,
                           Args&&... args /* NOLINT */) -> OutputIt {
    out = style.to_escape(out);
    out = std::vformat_to(
        out, fmt.get(), make_format_args(process_fmt_arg(args, abs(style))...));
    if (!style.is_null()) out = abs(null_style).to_escape(out);
    return out;
}

template <style_type StyleT, typename... Args>
inline auto format_impl(StyleT style,
                        std::format_string<Args...> fmt,
                        Args&&... args /* NOLINT */) -> std::string {
    auto buf = FormatBuf();
    format_to_impl(BufAppender(buf), style, fmt, std::forward<Args>(args)...);
    return {buf.data(), buf.size()};
}

template <style_type StyleT>
struct CtxStyle {
    constexpr CtxStyle(StyleT style, const OutputConfig& cfg)
        : style(style),
          cfg(&cfg) {}

    StyleT style;
    NotNull<const OutputConfig*> cfg;
};

template <style_type StyleT, typename T>
struct CtxStyled {
    constexpr CtxStyled(Styled<StyleT, T> styled,
                        AbsoluteStyle restore_style,
                        const OutputConfig& cfg)
        : styled(std::move(styled)),
          restore_style(restore_style),
          cfg(&cfg) {}

    Styled<StyleT, T> styled;
    AbsoluteStyle restore_style;
    NotNull<const OutputConfig*> cfg;
};

// Replace Styled<T> and style types
template <typename Arg>
constexpr auto process_fmt_arg_ctx(const Arg& arg,
                                   AbsoluteStyle& current_style,
                                   const OutputConfig& cfg) -> decltype(auto) {
    static_assert(!std::is_same_v<std::remove_cvref_t<Arg>, Reset>,
                  "'reset' cannot be used as a format argument with Printer."
                  "Use the reset() member function instead.");
    if constexpr (style_type<Arg>) {
        current_style = merge(current_style, arg);
        return CtxStyle(arg, cfg);
    } else if constexpr (styled<Arg>) {
        return CtxStyled(std::move(arg), current_style, cfg);
    } else {
        return arg;
    }
}

template <std::output_iterator<const char&> OutputIt, detail::style_type StyleT>
auto write_style(OutputIt out, StyleT style, const OutputConfig& cfg)
    -> OutputIt {
    if (!cfg.style_enabled()) return out;
    style = detail::fallback_color(style, cfg.color_mode());
    return style.to_escape(out);
}

#ifdef DECO_ENABLE_STD_PRINT

template <typename Stream, style_type StyleT, typename... Args>
inline auto print_impl(Stream& stream,
                       StyleT style,
                       std::format_string<Args...> fmt,
                       Args&&... args) {
    auto buffer = FormatBuf();
    format_to_impl(
        BufAppender(buffer), style, fmt, std::forward<Args>(args)...);
    std::print(stream, "{}", std::string_view(buffer.data(), buffer.size()));
}

template <typename Stream, style_type StyleT, typename... Args>
inline auto println_impl(Stream& stream,
                         StyleT style,
                         std::format_string<Args...> fmt,
                         Args&&... args) {
    auto buffer = FormatBuf();
    format_to_impl(
        BufAppender(buffer), style, fmt, std::forward<Args>(args)...);
    std::println(stream, "{}", std::string_view(buffer.data(), buffer.size()));
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
struct formatter<deco::detail::Styled<StyleT, T>>
    : deco::detail::formatter_t<T> {
    auto format(const deco::detail::Styled<StyleT, T>& styled,
                std::format_context& ctx) const {
        using namespace deco;

        ctx.advance_to(styled.style().to_escape(ctx.out()));
        ctx.advance_to(detail::formatter_t<T>::format(styled.value(), ctx));
        ctx.advance_to(abs(null_style).to_escape(ctx.out()));

        return ctx.out();
    }
};

template <deco::detail::style_type StyleT, typename T>
    requires deco::formattable<T>
struct formatter<deco::detail::FStyled<StyleT, T>>
    : deco::detail::formatter_t<T> {
    auto format(const deco::detail::FStyled<StyleT, T>& fstyled,
                std::format_context& ctx) const {
        using namespace deco;

        const auto& styled = fstyled.styled;

        ctx.advance_to(styled.style().to_escape(ctx.out()));
        ctx.advance_to(detail::formatter_t<T>::format(styled.value(), ctx));
        ctx.advance_to(fstyled.restore_style.to_escape(ctx.out()));

        return ctx.out();
    }
};

template <deco::detail::style_type StyleT>
struct formatter<deco::detail::CtxStyle<StyleT>> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(deco::detail::CtxStyle<StyleT> ctx_style,
                std::format_context& ctx) const {
        return deco::detail::write_style(
            ctx.out(), ctx_style.style, *ctx_style.cfg);
    }
};

template <deco::detail::style_type StyleT, typename T>
    requires deco::formattable<T>
struct formatter<deco::detail::CtxStyled<StyleT, T>>
    : deco::detail::formatter_t<T> {
    auto format(const deco::detail::CtxStyled<StyleT, T>& ctx_styled,
                std::format_context& ctx) const {
        using namespace deco::detail;

        const auto& styled = ctx_styled.styled;
        const auto& cfg = *ctx_styled.cfg;

        ctx.advance_to(write_style(ctx.out(), styled.style(), cfg));
        ctx.advance_to(formatter_t<T>::format(styled.value(), ctx));
        ctx.advance_to(write_style(ctx.out(), ctx_styled.restore_style, cfg));

        return ctx.out();
    }
};

} // namespace std

namespace deco {

// ╔═════════════════════════════════════════════════════════╗
// ║                         format                          ║
// ╚═════════════════════════════════════════════════════════╝

template <std::output_iterator<const char&> OutputIt, typename... Args>
inline auto format_to(OutputIt out,
                      Style style,
                      std::format_string<Args...> fmt,
                      Args&&... args) -> OutputIt {
    return detail::format_to_impl(out, style, fmt, std::forward<Args>(args)...);
}

template <std::output_iterator<const char&> OutputIt, typename... Args>
inline auto format_to(OutputIt out,
                      AbsoluteStyle abstyle,
                      std::format_string<Args...> fmt,
                      Args&&... args) -> OutputIt {
    return detail::format_to_impl(
        out, abstyle, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] inline auto format(Style style,
                                 std::format_string<Args...> fmt,
                                 Args&&... args) -> std::string {
    return detail::format_impl(style, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] inline auto format(AbsoluteStyle abstyle,
                                 std::format_string<Args...> fmt,
                                 Args&&... args) -> std::string {
    return detail::format_impl(abstyle, fmt, std::forward<Args>(args)...);
}

#ifdef DECO_ENABLE_STD_PRINT

// ╔═════════════════════════════════════════════════════════╗
// ║                          print                          ║
// ╚═════════════════════════════════════════════════════════╝

template <typename... Args>
inline void print(Style s, std::format_string<Args...> fmt, Args&&... args) {
    detail::print_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(FILE* f,
                  Style s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(FILE* f,
                  AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::ostream& os,
                  Style s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::ostream& os,
                  AbsoluteStyle s,
                  std::format_string<Args...> fmt,
                  Args&&... args) {
    detail::print_impl(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(Style s, std::format_string<Args...> fmt, Args&&... args) {
    detail::println_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl<FILE*>(stdout, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(FILE* f,
                    Style s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(FILE* f,
                    AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl<FILE*>(f, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::ostream& os,
                    Style s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl(os, s, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void println(std::ostream& os,
                    AbsoluteStyle s,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
    detail::println_impl(os, s, fmt, std::forward<Args>(args)...);
}

// ╔═════════════════════════════════════════════════════════╗
// ║                         Printer                         ║
// ╚═════════════════════════════════════════════════════════╝

class Printer {
  public:
    explicit Printer(const OutputConfig& config, std::ostream& os)
        : cfg_(&config),
          stream(&os) {}

    explicit Printer(const OutputConfig& config, std::FILE* f = stdout)
        : cfg_(&config),
          stream(f) {}

    auto set_base_style(Style s) -> Printer& {
        ctx_.set_base_style(s);
        return *this;
    }

    auto ensure_style() -> Printer& {
        ctx_.ensure_style();
        return *this;
    }

    auto set(Style s) -> Printer& {
        ctx_.merge_current_style(s);
        return *this;
    }

    auto push(Style s) -> Printer& {
        ctx_.push(s);
        return *this;
    }

    auto push(AbsoluteStyle s) -> Printer& {
        ctx_.push(s);
        return *this;
    }

    auto pop() -> Printer& {
        ctx_.pop();
        return *this;
    }

    auto reset() -> Printer& {
        ctx_.reset();
        return *this;
    }

    [[nodiscard]] auto current_style() const -> Style {
        return ctx_.current_abstyle().inner();
    }

    [[nodiscard]] auto base_style() const -> Style { return ctx_.base_style(); }

    /* ----- print ----- */

    template <typename... Args>
    auto print(std::format_string<Args...> fmt, Args&&... args) -> Printer& {
        this->print_impl(null_style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto print(Style style, std::format_string<Args...> fmt, Args&&... args)
        -> Printer& {
        this->print_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto print(AbsoluteStyle style,
               std::format_string<Args...> fmt,
               Args&&... args) -> Printer& {
        this->print_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto println(std::format_string<Args...> fmt, Args&&... args) -> Printer& {
        this->println_impl(null_style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto println(Style style, std::format_string<Args...> fmt, Args&&... args)
        -> Printer& {
        this->println_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args>
    auto println(AbsoluteStyle style,
                 std::format_string<Args...> fmt,
                 Args&&... args) -> Printer& {
        this->println_impl(style, fmt, std::forward<Args>(args)...);
        return *this;
    }

  private:
    using Stream = std::variant<detail::NotNull<std::FILE*>,
                                detail::NotNull<std::ostream*>>;

    template <std::output_iterator<const char&> OutputIt,
              detail::style_type StyleT>
    auto write_pending(OutputIt out, StyleT with) -> OutputIt {
        assert(ctx_.has_pending());
        auto pending = ctx_.consume_pending();
        if (!with.is_null()) pending.merge(with);
        return pending.visit(
            [this, out](detail::style_type auto style) constexpr {
                return detail::write_style(out, style, *cfg_);
            });
        return out;
    }

    template <std::output_iterator<const char&> OutputIt,
              detail::style_type StyleT,
              typename... Args>
    auto format_to(OutputIt out,
                   StyleT style,
                   std::format_string<Args...> fmt,
                   Args&&... args /* NOLINT */) -> OutputIt {
        AbsoluteStyle current_style =
            detail::merge(ctx_.current_abstyle(), style);
        if (ctx_.has_pending()) out = this->write_pending(out, style);
        else out = detail::write_style(out, style, *cfg_);

        out = std::vformat_to(
            out,
            fmt.get(),
            detail::make_format_args(
                detail::process_fmt_arg_ctx(args, current_style, *cfg_)...));

        if (current_style != ctx_.current_abstyle())
            out = detail::write_style(out, ctx_.current_abstyle(), *cfg_);

        return out;
    }

    template <detail::style_type StyleT, typename... Args>
    void print_impl(StyleT style,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
        auto buffer = detail::FormatBuf();
        this->format_to(detail::BufAppender(buffer),
                        style,
                        fmt,
                        std::forward<Args>(args)...);
        this->print_stream(buffer);
    }

    template <detail::style_type StyleT, typename... Args>
    void println_impl(StyleT style,
                      std::format_string<Args...> fmt,
                      Args&&... args) {
        auto buffer = detail::FormatBuf();
        this->format_to(detail::BufAppender(buffer),
                        style,
                        fmt,
                        std::forward<Args>(args)...);
        this->println_stream(buffer);
    }

    void print_stream(const detail::FormatBuf& buf) {
        if (auto* f = std::get_if<detail::NotNull<std::FILE*>>(&stream)) {
            std::print(*f, "{}", std::string_view(buf.data(), buf.size()));
        } else if (auto* os =
                       std::get_if<detail::NotNull<std::ostream*>>(&stream)) {
            std::print(**os, "{}", std::string_view(buf.data(), buf.size()));
        }
    }

    void println_stream(const detail::FormatBuf& buf) {
        if (auto* f = std::get_if<detail::NotNull<std::FILE*>>(&stream)) {
            std::println(*f, "{}", std::string_view(buf.data(), buf.size()));
        } else if (auto* os =
                       std::get_if<detail::NotNull<std::ostream*>>(&stream)) {
            std::println(**os, "{}", std::string_view(buf.data(), buf.size()));
        }
    }

    detail::StyleContext ctx_;

    detail::NotNull<const OutputConfig*> cfg_;
    Stream stream;
};

/// Creates a `Printer` links to `stdout` and global output config.
inline auto stdout_printer() -> Printer {
    return Printer(global_cfg, stdout);
}

/// Creates a `Printer` links to `stdout` and global output config.
inline auto stderr_printer() -> Printer {
    return Printer(global_cfg, stderr);
}

#endif // DECO_ENABLE_STD_PRINT

} // namespace deco

#ifdef DECO_ENABLE_STD_PRINT
#undef DECO_ENABLE_STD_PRINT
#endif

#endif // !DECOTERM_FORMAT_HPP
