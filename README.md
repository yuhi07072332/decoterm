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

// Same as fmt::styled() from fmtlib
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

- Cross-platform (Windows, Linux, MacOS, etc.)
- Simple API syntax inspired by [{fmt}](https://github.com/fmtlib/fmt)
- Powerful output state management
- `std::format`, `std::print`(C++23) support

## Getting Started

### Single-header Mode

Just download [single-include/decoterm/decoterm.hpp](single-include/decoterm/decoterm.hpp) and add to your *include directories*.
> Alternatively, you can use [include/decoterm/style.hpp](include/decoterm/style.hpp) for **minimal style output functionality**.
> See [here](#header-overview) for more information.

### Header-only Mode

Copy `include/decoterm` folder to your *include directories* and use whatever header files you want.
See [Header Overview](#header-overview) for more information.

## Dependencies

The library itsself needs atleast **C++20**, examples and tests require `std::print`(**C++23** or above)to compile.

## Header Overview

```
               ┌───────────┐            
               │ style.hpp │  Minimal API       
               └───────────┘  [Color, Style, styled(), ostream output operator]
                     ▲            
                     │                  
             ┌───────┴────────┐         
             │ color_info.hpp │  Dataset of terminal colors 
             └────────────────┘  [color names for XTerm 256 colors]
                     ▲                  
                     │                  
            ┌────────┴─────────┐           
            │    output.hpp    │    Advanced style output functionalityies   
            └──────────────────┘    [StyledOstream]       
               ▲            ▲              
               │            │              
     ┌─────────┴──┐       ┌─┴────────────┐
     │ format.hpp │       │ terminal.hpp │ Cross-platform terminal utilities
     └────────────┘       └──────────────┘
std::format, std::print support          
[formatters, StyledFormat]

```

> [!NOTE]
> Currently, `color_info.hpp` only provides color names and not included by any other headers.
> This will contain terminal color informations for `output.hpp` to implement **Color fallback**
> function in the future.

## Development

### Building examples and tests

> [!NOTE]
> Examples and tests uses `std::print`, so building these requires atleast **C++23**, GCC 14, Clang 17, MSVC 19.37 to work.

```sh
cmake -Bbuild \
    -DDECOTERM_BUILD_EXAMPLES=on \
    -DDECOTERM_BUILD_TESTS=on \

cmake --build build
```

To run the tests:
```sh
ctest --test-dir build
```

### Enable `pre-commit` hook

This repository uses [pre-commit](https://pre-commit.com/) to generate the single-header file.

```sh
pip install pre-commit
pre-commit install
```

## Todo

- [ ] Color fallback
- [ ] `fmtlib` support (maybe)
- [ ] Windows API fallback for Windows8 or lower (maybe)
- [ ] link (maybe)
