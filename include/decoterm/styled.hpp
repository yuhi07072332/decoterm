// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_STYLED_HPP
#define DECOTERM_STYLED_HPP

#include "style.hpp"

#include <concepts>
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace deco {

namespace detail {

/* ----- forward declarations ----- */

/// @brief A wrapper owns a rvalue or seferences a lvalue.
template <typename T> struct StorageRef;

template <typename T> struct is_storage_ref;

// Whether remove_cvref_t<T> is a Storage.
template <typename T>
concept storage_ref = is_storage_ref<std::remove_cvref_t<T>>::value;

} // namespace detail

template <detail::storage_ref, detail::outputable_style>
struct StyledRef;

class StyleOutputState;

namespace detail {

/* ----- global style output context ----- */

inline StyleOutputState* g_style_output_context = nullptr;

/* ----- type traits & concepts ----- */

// Whether T has operator<<(std::ostream&, T)
template <typename T>
concept ostream_outputable = requires(std::ostream& os, const T& value) {
    { os << value } -> std::same_as<std::ostream&>;
};

template <typename T> struct is_storage_ref : std::false_type {};

template <typename T> struct is_storage_ref<StorageRef<T>> : std::true_type {};

template <typename T> struct is_styled_ref : std::false_type {};

template <storage_ref T, typename U>
struct is_styled_ref<StyledRef<T, U>> : std::true_type {};

template <typename T>
concept styled_ref = is_styled_ref<std::remove_cvref_t<T>>::value;

/* ----- Storage ----- */

/// @brief Specialization of Storage for rvalue.
template <typename T>
struct StorageRef {
    using value_type = T;

    static_assert(!std::is_reference_v<T>);

    constexpr explicit StorageRef(T value) : value_(std::move(value)) {}
    constexpr auto get() const -> const T& { return value_; }

  private:
    T value_;
};

/// @brief Specialization of Storage for const lvalue reference.
template <typename T>
struct StorageRef<const T&> {
    using value_type = T;

    constexpr explicit StorageRef(const T& value) : value_(value) {}
    constexpr auto get() const -> const T& { return value_; }

  private:
    const T& value_;
};

/// @brief Make Storage from lvalue.
template <typename T>
    requires(!std::is_rvalue_reference_v<T>)
inline constexpr auto make_storage(const T& value) -> StorageRef<const T&> {
    return StorageRef<const T&>(value);
}

/// @brief Make Storage from rvalue.
template <typename T>
    requires(!std::is_lvalue_reference_v<T>)
inline constexpr auto make_storage(T&& value) -> StorageRef<const T> {
    return StorageRef<const T>(std::move(value));
}

/// Cannot make Storage from const rvalue reference.
template <typename T>
inline constexpr auto make_storage(const T&&) = delete;

/* ----- StyleStack ----- */

/// @brief represents a nullable, single or multiple(vector) AbsoluteStyle
/// stack.
class StyleStack {
  public:
    // single by default
    StyleStack() = default;

    [[nodiscard]]
    auto top() const -> std::optional<AbsoluteStyle> {
        if (is_multiple_) {
            if (multiple_.empty()) return std::nullopt;
            else return multiple_.back();
        }
        return single_;
    }

    [[nodiscard]]
    auto is_multiple() const -> bool {
        return is_multiple_;
    }

    void push(AbsoluteStyle style) {
        if (is_multiple_) {
            multiple_.push_back(style);
        } else single_ = style;
    }

    void pop() {
        if (is_multiple_ && !multiple_.empty()) multiple_.pop_back();
        else single_ = std::nullopt;
    }

    void clear() {
        if (is_multiple_) multiple_.clear();
        else single_ = std::nullopt;
    }

    void to_multiple() {
        if (is_multiple_) return;
        multiple_.clear();
        if (single_) multiple_.push_back(*single_);
        is_multiple_ = true;
    }

    void to_single() {
        if (!is_multiple_) return;
        if (!multiple_.empty()) single_ = multiple_.back();
        else single_ = std::nullopt;
        is_multiple_ = false;
    }

  private:
    std::vector<AbsoluteStyle> multiple_;
    std::optional<AbsoluteStyle> single_;
    bool is_multiple_ = false;
};

} // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                         Styled                          ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A wrapper for styling a value.
template <detail::storage_ref StorageType, detail::outputable_style StyleType>
struct StyledRef : public StorageType {
    constexpr explicit StyledRef(StorageType storage, StyleType style)
        : StorageType(std::move(storage)),
          style_(style) {}

    constexpr auto style() const -> StyleType { return style_; }

  private:
    StyleType style_;
};

/// @brief Create a StyledValue.
template <typename T, detail::outputable_style StyleType>
inline constexpr auto styled(T&& value, StyleType style) {
    return StyledRef(detail::make_storage(std::forward<T>(value)), style);
}

template <detail::ostream_outputable T, detail::outputable_style StyleType>
inline auto
operator<<(std::ostream& os,
           const StyledRef<detail::StorageRef<T>, StyleType>& styled)
    -> std::ostream& {
    os << styled.style() << styled.get() << reset;
    return os;
}

// ╔═════════════════════════════════════════════════════════╗
// ║                    StyleOutputState                     ║
// ╚═════════════════════════════════════════════════════════╝

/// @brief A class representing Style output state.
class StyleOutputState {
  public:
    StyleOutputState() = default;

    ~StyleOutputState() {
        if (detail::g_style_output_context == this)
            detail::g_style_output_context = nullptr;
    }

    // ----- options -----

    auto enable_style(bool enable) -> StyleOutputState& {
        style_enabled_ = enable;
        return *this;
    }

    auto enable_color_fallback(bool enable) -> StyleOutputState& {
        color_fallback_enabled_ = enable;
        return *this;
    }

    auto base_style(Style base) -> StyleOutputState& {
        base_style_ = absolute(base);
        return *this;
    }

    auto enable_nesting(bool enable) -> StyleOutputState& {
        if (enable) stack_.to_multiple();
        else stack_.to_single();
        return *this;
    }

    // ----- observe -----

    [[nodiscard]]
    auto base_style() const -> Style {
        return base_style_.style;
    }

    [[nodiscard]]
    auto nesting_enabled() const -> bool {
        return stack_.is_multiple();
    }

    [[nodiscard]]
    auto style_enabled() const -> bool {
        return style_enabled_;
    }

    [[nodiscard]]
    auto color_fallback_enabled() const -> bool {
        return color_fallback_enabled_;
    }

    [[nodiscard]]
    auto current_style() const -> AbsoluteStyle {
        return stack_.top().value_or(base_style_);
    }

  protected:
    void pop_style() { stack_.pop(); }

    void reset_style() { stack_.clear(); }

    void push_style(AbsoluteStyle style) { stack_.push(style); }

    void push_style(Style style) {
        stack_.push(absolute(current_style().style | style));
    }

    AbsoluteStyle base_style_ = absolute(default_style);

    bool style_enabled_ = true;
    bool color_fallback_enabled_ = false;

  private:
    detail::StyleStack stack_;
};

// ╔═════════════════════════════════════════════════════════╗
// ║                      StyledOstream                      ║
// ╚═════════════════════════════════════════════════════════╝

struct style_pop_t {};
inline constexpr style_pop_t pop;

/// @brief A stateful lightweight writer over an existing std::ostream.
/// @warning The passed std::ostream object must outlive this object.
class StyledOstream : public StyleOutputState {
  public:
    StyledOstream(std::ostream& os) : ostream_(os) {}
    StyledOstream(const StyleOutputState& state, std::ostream& os)
        : StyleOutputState(state),
          ostream_(os) {}

    template <detail::ostream_outputable T>
        requires(!detail::styled_ref<T>)
    friend auto operator<<(StyledOstream& out, T&& value) -> StyledOstream& {
        using value_type = std::remove_cvref_t<T>;
        out.check_context();

        if constexpr (detail::outputable_style<value_type>) {
            if constexpr (std::is_same_v<value_type, Style>
                          || std::is_same_v<value_type, AbsoluteStyle>)
                out.push_style(value);
            out.output_style(value);
        } else if constexpr (std::is_same_v<value_type, style_reset_t>) {
            out.reset_style();
            out.output_style(out.current_style());
        } else {
            out.ostream_ << std::forward<T>(value);
        }
        return out;
    }

    /// @brief ostream operator for pop
    friend auto operator<<(StyledOstream& out, style_pop_t) -> StyledOstream& {
        out.check_context();
        out.pop_style();
        if (out.style_enabled_) out.ostream_ << out.current_style();
        return out;
    }

    /// @brief ostream operator for styled()

    template <detail::ostream_outputable T, detail::outputable_style StyleType>
    friend auto
    operator<<(StyledOstream& out,
               const StyledRef<detail::StorageRef<T>, StyleType>& styled)
        -> StyledOstream& {
        out.check_context();
        out.output_style(styled.style());
        out.ostream_ << styled.get();
        out.ostream_ << out.current_style();
        return out;
    }

    auto ostream() const -> std::ostream& { return ostream_; }

  private:
    void check_context() {
        if (detail::g_style_output_context
            != static_cast<StyleOutputState*>(this)) {
            output_style(current_style());
            detail::g_style_output_context = this;
        }
    }

    template <detail::outputable_style StyleType>
    void output_style(StyleType style) {
        if (style_enabled_) ostream_ << style;
    }

    std::ostream& ostream_;
};

inline auto styled_out(std::ostream& os) -> StyledOstream {
    return StyledOstream(os);
}

} // namespace deco

#endif // !DECOTERM_STYLED_HPP
