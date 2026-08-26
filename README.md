# decoterm

A simple header-only C++20 library for decorating terminal output.

## Quick Start

See the full example: [quickstart.cpp](examples/quickstart.cpp)

```cpp
// -- 1. Basic Style Output

// combine `Style`s with `|`

const deco::Style error_style = 
    deco::bold | deco::invert | deco::fg(deco::bright_red);
const deco::Style expected_style = 
    deco::bold | deco::fg(deco::rgb(238, 212, 159));

// use `Style`s like I/O manipulators

std::cout << error_style << "assertion failed " << deco::reset
    << "at ";

// use `Style`s as a function to decorate values

// -- same as `std::cout << deco::italic << "main.cpp" << deco::reset`
std::cout << deco::italic("main.cpp");

// -- this can also store multiple values.
std::cout << ":" << deco::bold(116) << ":   "
    << expected_style("expected ", deco::underline(3.14))
    << ", "
    << fg(deco::bright_magenta)("got ", deco::underline(2.71))
    << "\n\n";

// StyledOstream: ostream wrapper with style output states.

auto out = deco::StyledOstream(std::cout);

out.set_base_style(deco::bg(deco::rgb(32, 32, 32)))
   .enable_style(false)

out << deco::fg(deco::colors::medium_turquoise) << "style disabled!\n";

out.enable_style(true);
out << "style enabled!\n";
```

## Features

- Cross-platform support for Windows, Linux, macOS and other platforms
- Simple API inspired by [{fmt}](https://github.com/fmtlib/fmt)
- Powerful output state management
- `std::format`, `std::print`(C++23) support
- Performance comparable to raw ANSI escape code output

## Getting Started

### Single-header Mode

Just download [single-include/decoterm/decoterm.hpp](single-include/decoterm/decoterm.hpp)
and add it to your *include directories*.
> Alternatively, you can use [include/decoterm/style.hpp](include/decoterm/style.hpp)
> if you only need the minimal style output functionality.

### Header-only Mode

Clone this repository and copy the `include/decoterm` directory into your *include directories* and use whatever headers you need.
See [Header Overview](#header-overview) for more information.

## Requirements

The library itself requires atleast **C++20**. The examples and tests use `std::print` and therefore require **C++23** or later.

## Header Overview

```
                     ┌───────────┐    Minimal API                           
                     │ style.hpp │    [Color, Style, styled(),
                     └───────────┘     ostream output operator]             
                           ▲                                                
                           │                                                
 ┌────────────────┐        │                                                
 │ color_info.hpp │        │                                                
 └────────────────┘        │                                                
          ▲                │                                                
          │                │                                                
          │      ┌────────────────────┐  Advanced style output functionality
          └──────│     output.hpp     │  [StyledOstream]                    
                 └────────────────────┘                                     
                   ▲                ▲                                       
                   │                │                                       
        ┌──────────┴─┐             ┌┴─────────────┐  Cross-platform terminal
        │ format.hpp │             │ terminal.hpp │  utilities              
        └────────────┘             └──────────────┘ 
std::format and std::print support                                          
[formatters, StyledFormat]                                                  
```

> [!NOTE]
> Currently, `color_info.hpp` only provides color names and is not included by any other header.
> It will contain terminal color information used by `output.hpp` to implement **Color fallback** in the future.

## Development

### Building examples and tests

> [!NOTE]
> The examples and tests use `std::print`, so they require at least **C++23**, GCC 14, Clang 17, MSVC 19.37 to work.

```sh
cmake -Bbuild \
    -DDECOTERM_BUILD_EXAMPLES=on \
    -DDECOTERM_BUILD_TESTS=on

cmake --build build
```

To run the tests:
```sh
ctest --test-dir build
```

### Enabling `pre-commit` hook

This repository uses [pre-commit](https://pre-commit.com/) to generate the single-header file.

```sh
pip install pre-commit
pre-commit install
```

## Todo

- [ ] Color fallback
- [ ] Hyperlink
- [ ] Cmake package(maybe)
- [ ] `fmtlib` support (maybe)
- [ ] Windows API fallback for Windows 8 or lower (maybe)
