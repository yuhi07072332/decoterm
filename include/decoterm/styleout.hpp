// SPDX-License-Identifier: MIT
//
// Copyright (c) 2026 Yuhi0707
//
// This file is part of the decoterm library.
// For license information, see decoterm.hpp.

#ifndef DECOTERM_STYLEOUT_HPP
#define DECOTERM_STYLEOUT_HPP

#include "style.hpp"

#include <ostream>

namespace deco {

/// @brief A lightweight stateful writer over an existing std::ostream.
class StyleOut {
public:
    StyleOut(std::ostream& os) : os_(os) {}

private:
    std::ostream& os_;
};

} // namespace deco

#endif // !DECOTERM_STYLEOUT_HPP
