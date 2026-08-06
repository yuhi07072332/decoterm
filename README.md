# decoterm

A simple header-only C++20 library for decorating terminal output.

## Quick Start

See the full example: [quickstart.cpp](examples/quickstart.cpp)

```cpp
// -- 1. Basic Style Output

std::cout << bold << "Hello ,"
          << fg(bright_green) << "world!"
          << reset << "\n\n";

// Style composition
const Style warning_style = bold | fg(rgb(0xfff2b2));

// Similar to fmt::styled() from fmtlib
std::cout << styled("warning: ", warning_style)
          << "Unused variable 'x'"
          << styled("[-Wunused-variable]", warning_style)
          << '\n';

// -- 2. Advanced stateful output

// StyledOstream: ostream wrapper with style output states.
StyledOstream sout = styled_out(std::cout)
    .set_base_style(bg(rgb(32, 32, 32)))
    //Enable style output only when stdout is a TTY.
    .enable_style(terminal::is_stdout_tty())
    .enable_nesting();

// Style nesting
sout << "base style "
          << fg(bright_blue) << "{ blue "
              << italic << "{ italic blue "
                  << fg(bright_red) << "{ italic red } "
              << pop << "italic blue } "
          << pop << "blue } "
     << pop << "base style";

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
- [ ] link
- [ ] Cmake package(maybe)
- [ ] `fmtlib` support (maybe)
- [ ] Windows API fallback for Windows 8 or lower (maybe)
