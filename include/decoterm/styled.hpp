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

/// @brief A wrapper owns a rvalue or references lvalue by owning
/// std::reference_wrapper.
template <typename T> struct Storage;

/* ----- type traits & concepts ----- */

// Whether T has operator<<(std::ostream&, T)
template <typename T>
concept ostream_outputable = requires(std::ostream& os, T&& value) {
    { os << std::forward<T>(value) } -> std::same_as<std::ostream&>;
};

template <typename T> struct is_storage : std::false_type {};

template <typename T> struct is_storage<Storage<T>> : std::true_type {};

// Whether T is a Storage.
template <typename T>
concept storage = is_storage<T>::value;

/* ----- Storage ----- */

/// @brief Specialization of Storage for rvalue.
template <typename T>
struct Storage {
    static_assert(!std::is_reference_v<T>);
    static_assert(!std::is_const_v<T>);

    constexpr explicit Storage(T value) : value_(std::move(value)) {}

    constexpr auto get() -> T& { return value_; }
    constexpr auto get() const -> const T& { return value_; }

  private:
    T value_;
};

/// @brief Specialization of Storage for lvalue reference.
template <typename T>
struct Storage<T&> {
    constexpr explicit Storage(T& value) : value_(value) {}
    constexpr auto get() const -> T& { return value_; }

  private:
    T& value_;
};

/// @brief Make Storage from lvalue.
template <typename T>
    requires(!std::is_rvalue_reference_v<T>)
inline constexpr auto make_storage(T& value) -> Storage<T&> {
    return Storage<T&>(value);
}

/// @brief Make Storage from rvalue.
template <typename T>
    requires(!std::is_lvalue_reference_v<T>)
inline constexpr auto make_storage(T&& value) -> Storage<T> {
    return Storage<T>(std::move(value));
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
template <detail::storage StorageType, detail::outputable_style StyleType>
struct StyledValue : public StorageType {
    constexpr explicit StyledValue(StorageType storage, StyleType style)
        : StorageType(std::move(storage)),
          style_(style) {}

    constexpr auto style() const -> StyleType { return style_; }

  private:
    StyleType style_;
};

/// @brief Create a StyledValue.
template <typename T, detail::outputable_style StyleType>
inline constexpr auto styled(T&& value, StyleType style) {
    return StyledValue(detail::make_storage(std::forward<T>(value)), style);
}

template <detail::storage StorageType, detail::outputable_style StyleType>
inline auto operator<<(std::ostream& os,
                       StyledValue<StorageType, StyleType>& styled)
    -> std::ostream& {
    os << styled.style() << styled.get() << reset;
    return os;
}

template <detail::storage StorageType, detail::outputable_style StyleType>
inline auto operator<<(std::ostream& os,
                       const StyledValue<StorageType, StyleType>& styled)
    -> std::ostream& {
    os << styled.style() << styled.get() << reset;
    return os;
}

template <detail::storage StorageType, detail::outputable_style StyleType>
inline auto operator<<(std::ostream& os,
                       StyledValue<StorageType, StyleType>&& styled)
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

    auto base_style() const -> Style { return base_style_.style; }

    auto nesting_enabled() const -> bool { return stack_.is_multiple(); }

    auto style_enabled() const -> bool { return style_enabled_; }

    auto color_fallback_enabled() const -> bool {
        return color_fallback_enabled_;
    }

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
    bool is_first_output_ = true;

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
    friend auto operator<<(StyledOstream& so, T&& value) -> StyledOstream& {
        using value_type = std::remove_cvref_t<T>;
        if (so.is_first_output_) {
            so.output_style(so.base_style_);
            so.is_first_output_ = false;
        }

        if constexpr (detail::outputable_style<value_type>) {
            if constexpr (std::is_same_v<value_type, Style>
                          || std::is_same_v<value_type, AbsoluteStyle>)
                so.push_style(value);
            so.output_style(value);
        } else if constexpr (std::is_same_v<value_type, style_reset_t>) {
            so.reset_style();
            so.output_style(so.current_style());
        } else {
            so.ostream_ << std::forward<T>(value);
        }
        return so;
    }

    friend auto operator<<(StyledOstream& so, style_pop_t) -> StyledOstream& {
        so.pop_style();
        if (so.style_enabled_) so.ostream_ << so.current_style();
        return so;
    }

    template <detail::storage StorageType, detail::outputable_style StyleType>
    friend auto operator<<(StyledOstream& so,
                           StyledValue<StorageType, StyleType>& styled)
        -> StyledOstream& {
        so.output_style(styled.style());
        so.ostream_ << styled.get() << reset;
        so.reset_style();
        return so;
    }

    template <detail::storage StorageType, detail::outputable_style StyleType>
    friend auto operator<<(StyledOstream& so,
                           const StyledValue<StorageType, StyleType>& styled)
        -> StyledOstream& {
        so.output_style(styled.style());
        so.ostream_ << styled.get() << reset;
        so.reset_style();
        return so;
    }

    template <detail::storage StorageType, detail::outputable_style StyleType>
    friend auto operator<<(StyledOstream& so,
                           StyledValue<StorageType, StyleType>&& styled)
        -> StyledOstream& {
        so.output_style(styled.style());
        so.ostream_ << styled.get() << reset;
        so.reset_style();
        return so;
    }

    auto ostream() const -> std::ostream& { return ostream_; }

  private:
    template <detail::outputable_style StyleType>
    void output_style(StyleType style) {
        if (style_enabled_) ostream_ << style;
    }

    std::ostream& ostream_;
};

} // namespace deco

#endif // !DECOTERM_STYLED_HPP
