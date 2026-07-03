# decoterm

A simple header-only C++20 library for terminal styling.

## Feature

in `decoterm.hpp`:

- Cross-platform support **(wip, only linux currently)**
- Terminal system 16 colors, 256 colors and true color (from rgb or hsv) support
- Simple API and syntax (in my opinion)
- Style stack
- ~Some unnecessary optimizations~

in `terminal.hpp`:

- Auto disable style output if standard output is not terminal
- Detect color support and provide fallback for true color (wip)

in other headers:

- formatter support for std::format

## Quickstart

[](./example/quickstart.cpp)

## Installization


## Todo

- Detect color support & provide system color fallback for true color  **(wip)**

in other headers:

- formatter support for `std::format`
