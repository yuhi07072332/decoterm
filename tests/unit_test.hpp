#include <doctest.h>

#include "decoterm/style.hpp"

#include <string>

// NOLINTBEGIN

namespace {
std::string printable_csi(const std::string& esc) {
    assert(esc.starts_with("\x1b["));
    return "CSI" + esc.substr(2);
}

}

namespace deco {

// Stringification for custom types

doctest::String toString(Color c) {
    return printable_csi(c.debug_string());
}

doctest::String toString(Style s) {
    return printable_csi(s.to_escape());
}

doctest::String toString(AbsoluteStyle a) {
    return printable_csi(a.to_escape());
}

}

// NOLINTEND
