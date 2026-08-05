# decoterm

A simple header-only C++20 (or above) library for decorating terminal output

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
- `<format>` support

## Getting Started

### Single-header Mode

Just download [single-include/decoterm/decoterm.hpp](single-include/decoterm/decoterm.hpp) and add to your *include directories*.
> Alternatively, you can use [include/decoterm/style.hpp](include/decoterm/style.hpp) for minimal style functionality. See [here](#header-overview) for more information.

### Header-only Mode

Copy `include/decoterm` folder to your *include directories* and use whatever header files you want.
See [Header Overview](#header-overview) for more information.

## Header Overview

```
               ┌───────────┐            
               │ style.hpp │  Minimal API       
               └───────────┘  (Color, Style, styled(), ostream output operator)
                     ▲            
                     │                  
             ┌───────┴────────┐         
             │ color_info.hpp │  Dataset of terminal colors 
             └────────────────┘  (color names for XTerm 256 colors)
                     ▲                  
                     │                  
            ┌────────┴─────────┐           
            │    output.hpp    │    Advanced style output functionalityies   
            └──────────────────┘    (StyledOstream)       
               ▲            ▲              
               │            │              
     ┌─────────┴──┐       ┌─┴────────────┐
     │ format.hpp │       │ terminal.hpp │ Cross-platform terminal utilities
     └────────────┘       └──────────────┘
std::format support          
(formatters, StyledFormat)

```

## Todo

- [ ] color fallback
- [ ] link (maybe)
