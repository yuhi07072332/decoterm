# decoterm

A header-only C++20 library for decorating terminal output.


## Quickstart

See the full example: [quickstart.cpp](examples/quickstart.cpp)

```cpp

// -- 1. Basic Style Output

std::cout << bold << "Hello ,"
          << fg(bright_green) << "world!"
          << reset      // reset to default style
          << "\n\n";

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

- Header-only C++20 library
- Cross-platform support (Windows, Linux, MacOS, etc.)
- 
- Simple API syntax (in my opinion)
- Stateful `StyledOstream` with optional style output, base style, context
  tracking, and nested style support
- Terminal helpers for stdout/stderr TTY checks, color support detection, and
  Windows virtual terminal mode setup
- `<format>` support for styles and styled values

## Installation


## Todo

- [ ] color fallback
- [ ] link (maybe)
