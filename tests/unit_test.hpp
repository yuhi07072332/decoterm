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

}

namespace deco {

// Stringification for custom types

inline doctest::String toString(Color c) {
    return printable_csi(c.debug_string());
}

inline doctest::String toString(Style s) {
    return printable_csi(s.to_escape());
}

inline doctest::String toString(AbsoluteStyle a) {
    return printable_csi(a.to_escape());
}

}

// NOLINTEND
