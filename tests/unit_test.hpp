#include <doctest.h>

#include "decoterm/style.hpp"

#include <string>

// NOLINTBEGIN

namespace {

inline std::string printable_csi(const std::string& esc) {
    assert(esc.starts_with("\x1b["));
    return (deco::bold | deco::fg(deco::cyan)).to_escape() 
    + ("CSI") + deco::abs(deco::null_style).to_escape()
    + esc.substr(2);
}

struct EscapeSeq {
    std::string str;

    EscapeSeq(std::string str) : str(std::move(str)) {}

    auto operator==(const EscapeSeq& other) -> bool {
        return str == other.str;
    }
};

template <typename StyleT>
auto to_escape(StyleT style) -> EscapeSeq {
    return style.to_escape();
}

// Stringification for escape sequence.
inline auto toString(const EscapeSeq& s) -> doctest::String {
    return printable_csi(s.str);
}

}



namespace deco {

// Stringification for custom types.

inline auto toString(Color c) -> doctest::String {
    return printable_csi(c.debug_string());
}

inline auto toString(Style s) -> doctest::String {
    return printable_csi(s.to_escape());
}

inline auto toString(AbsoluteStyle a) -> doctest::String {
    return printable_csi(a.to_escape());
}

} // namespace deco

// NOLINTEND
