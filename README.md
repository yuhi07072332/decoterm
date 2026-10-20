# decoterm

A simple header-only C++20 library for decorating terminal output.

## Quick Start

[std::print version](examples/quickstart-fmt.cpp)  

```cpp
using namespace deco::colors;
using namespace deco::styles;

// combine styles using `|`
constexpr deco::Style info = bold | fg(bright_cyan);
constexpr deco::Style warning = bold | fg(yellow);
constexpr deco::Style error = bold | fg(rgb(255, 85, 85));

// output `Style`
std::cout << info << "[INFO]" << deco::reset << " server started\n";

// Styled values similar to {fmt} library.
// You can also use `value | style` instead of `styled(value, style)`.
std::cout << deco::styled("[WARN]", warning) << " cache is nearly full\n";
std::cout << ("[ERROR]" | error) << " request failed: " << (503 | bold) << '\n';

// `init()` will detect terminal environment and initialize the global config.
deco::TerminalInfo term = deco::init();

// Modify the global config to disable style output if stdout is redirected.
deco::config().enable_style(term.is_stdout_tty);

// `OStream`: a `std::ostream` wrapper that tracks style context and uses
//  the global config.
// `stdout_ostream()` wraps `std::cout` with the global config.
deco::OStream log = deco::stdout_ostream();

// nested style
log << deco::push(info)
    << "[INFO] processed "
    << (128 | bold | fg(bright_green))
    << " records in "
    << deco::push(underline | fg(yellow))
    << 42 << " ms"
    << deco::pop
    << '\n'
    << deco::pop;
```

Preview:  

![preview](https://github.com/user-attachments/assets/406b696e-0612-4e63-8315-be558df62c46)

## Features

- Cross-platform support for Windows, Linux, macOS and other platforms
- Simple API syntax
- `std::format`, `std::print` support

## Getting Started

### Single-header Mode

Just download [single-include/decoterm/decoterm.hpp](single-include/decoterm/decoterm.hpp)
and add it to your *include directories*.
> Alternatively, you can use [include/decoterm/deco.hpp](include/decoterm/deco.hpp)
> if you only need the minimal style output functionality.

### Header-only Mode

Clone this repository and copy the `include/decoterm` directory into your *include directories* and use whatever headers you need.
See [Header Overview](#header-overview) for more information.

## Requirements

The library itself requires atleast **C++20**. The examples and tests use `std::print` and therefore require **C++23** or later.

## Header Overview

```
                     ┌───────────┐    Minimal API                           
                     │ deco.hpp  │    [Color, Style, styled()]
                     └───────────┘
                           ▲                                                
                           │                                                
 ┌────────────────┐        │                                                
 │ color_info.hpp │        │                                                
 └────────────────┘        │                                                
          ▲                │                                                
          │                │                                                
          │      ┌────────────────────┐  Advanced style output functionality
          └──────│     output.hpp     │  [Ostream]                    
                 └────────────────────┘                                     
                   ▲                ▲                                       
                   │                │                                       
        ┌──────────┴─┐             ┌┴─────────────┐  Cross-platform terminal
        │ format.hpp │             │ terminal.hpp │  utilities              
        └────────────┘             └──────────────┘ 
std::format and std::print support                                          
[formatters, Printer]                                                  
```

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
- [ ] Windows API fallback for Windows 8 or lower (maybe)
