// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yuhi0707
// This file is part of the decoterm library.
// For license information, see style.hpp.

#ifndef DECOTERM_FORMAT_HPP
#define DECOTERM_FORMAT_HPP

#include "output.hpp"
#include "style.hpp"

#ifdef DECOTERM_DETAIL_ENABLE_STD_FORMAT

#include <format>
#include <iterator>
#include <version>

#ifdef DECOTERM_DETAIL_ENABLE_STD_PRINT
#   include <cstdio>
#   include <print>
#endif

namespace deco {

namespace detail {
template <std::output_iterator<const char&> OutputIt,
          style_type StyleT,
          typename... Args>
inline auto format_to_impl(OutputIt out,
                           StyleT style,
                           std::format_string<Args...> fmt,
                           Args&&... args /* NOLINT */) -> OutputIt {
    out = style.to_escape(out);
    out = std::vformat_to(
        out,
        fmt.get(),
        make_format_args(process_fmt_arg(args, abs(style), nullptr)...));
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

#ifdef DECOTERM_DETAIL_ENABLE_STD_PRINT

template <typename Stream, style_type StyleT, typename... Args>
inline auto print_impl(Stream&& stream /* NOLINT */,
                       StyleT style,
                       std::format_string<Args...> fmt,
                       Args&&... args) {
    auto buffer = FormatBuf();
    format_to_impl(
        BufAppender(buffer), style, fmt, std::forward<Args>(args)...);
    std::print(stream, "{}", std::string_view(buffer.data(), buffer.size()));
}

template <typename Stream, style_type StyleT, typename... Args>
inline auto println_impl(Stream&& stream /* NOLINT */,
                         StyleT style,
                         std::format_string<Args...> fmt,
                         Args&&... args) {
    auto buffer = FormatBuf();
    format_to_impl(
        BufAppender(buffer), style, fmt, std::forward<Args>(args)...);
    std::println(stream, "{}", std::string_view(buffer.data(), buffer.size()));
}

#endif // DECOTERM_DETAIL_ENABLE_STD_PRINT

} // namespace detail

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

#ifdef DECOTERM_DETAIL_ENABLE_STD_PRINT

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

#endif // DECOTERM_DETAIL_ENABLE_STD_PRINT

} // namespace deco

#endif // DECOTERM_DETAIL_ENABLE_STD_FORMAT

#endif // !DECOTERM_FORMAT_HPP
