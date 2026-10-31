---
tocdepth: 3
---

# API Reference

(header_overview)=
## Headers

The `decoterm.hpp` convenience header includes every public header. Include an
individual header when you need only a specific part of the library.

**Header dependencies**

:::{md-mermaid}
flowchart TD
    deco[deco.hpp] --> output[output.hpp]
    output --> sync[sync.hpp]
    output --> format[format.hpp]
    output --> terminal[terminal.hpp]
    color256[color256.hpp]
:::

- {ref}`deco.hpp <deco_hpp>`: Defines the core types, including
  {cpp:struct}`deco::Color` and {cpp:struct}`deco::Style`.
- {ref}`output.hpp <output_hpp>`: Defines output configuration and styled
  writers.
- {ref}`format.hpp <format_hpp>`: Defines wrappers for
  {cpp:func}`std::format` and {cpp:func}`std::print`.
- {ref}`terminal.hpp <terminal_hpp>`: Defines terminal-detection and
  initialization utilities.
- {ref}`sync.hpp <sync_hpp>`: Defines {cpp:class}`deco::OSyncStream`.
- {ref}`color256.hpp <color256_hpp>`: Defines named xterm 256-color indices.

## Configuration macros

#### `DECOTERM_NO_FORMAT`
Disables APIs that require `<format>` or `<print>`. This can reduce compile time.

::: {note}
You need to define this macro before including any decoterm header.
:::

(deco_hpp)=
## Core types and styles

Provided by `deco.hpp`.

### Color

> :::{doxygenstruct} deco::Color

> :::{doxygenenum} deco::ColorType

### Colors

The inline `deco::colors` namespace provides helper functions and predefined
constants.

Helper functions create {cpp:struct}`deco::Color` values:

> ::: {doxygengroup} ColorHelpers
> :content-only:
> :::

Predefined {cpp:struct}`deco::Color` constants:

> ::: {doxygengroup} ColorConstants
> :content-only:
> :::

### Style

> :::{doxygenstruct} deco::Style

> :::{doxygenstruct} deco::AbsoluteStyle

### Styles

The inline `deco::styles` namespace provides helper functions and predefined
constants.

Helper functions create {cpp:struct}`deco::Style` values:

> ::: {doxygengroup} StyleHelpers
> :content-only:
> :::

Predefined {cpp:struct}`deco::Style` constants:

> ::: {doxygengroup} StyleConstants
> :content-only:
> :::

### Styled values

Use {cpp:func}`deco::styled` to associate a value with a style
(similar to [{fmt}](https://fmt.dev)).

> :::{doxygenfunction} styled(const T&, Style)
> :::
> :::{doxygenfunction} styled(const T&, AbsoluteStyle)

Alternatively, use `value | style`.

> :::{doxygenfunction} operator|(const T&, Style)
> :::
> :::{doxygenfunction} operator|(const T&, AbsoluteStyle)


(output_hpp)=
## Styled output

Provided by `output.hpp`.

### Output configuration

Pass {cpp:class}`deco::OutputConfig` to a
{ref}`styled writer <api_styled_writers>` to control its output.

> :::{doxygenclass} deco::OutputConfig

Create an {cpp:class}`deco::OutputConfig` or use the global configuration
returned by {cpp:func}`deco::config`.

> :::{doxygenfunction} deco::config

(api_styled_writers)=
### Styled writers

The styled writers are {cpp:class}`deco::OStream` and
{cpp:class}`deco::Printer`.

> :::{doxygenclass} deco::OStream

> :::{doxygenfunction} deco::stdout_ostream

> :::{doxygenfunction} deco::stderr_ostream

::: {note}
`deco::Printer` requires `<print>` support.
:::

> :::{doxygenclass} deco::Printer

> :::{doxygenfunction} deco::stdout_printer

> :::{doxygenfunction} deco::stderr_printer

(terminal_hpp)=
## Terminal utilities

Provided by `terminal.hpp`.

### Get terminal information

> :::{doxygenstruct} deco::TerminalInfo

> :::{doxygenfunction} deco::terminal_info

### Initialize output

> :::{doxygenfunction} deco::init

(format_hpp)=
## Format and print functions

Provided by `format.hpp`. The formatting functions are available only when the

::: {note}
{cpp:func}`deco::print` and {cpp:func}`deco::println` requires `<print>` support.
:::

### `deco::format`

Use these functions to format text with a {cpp:struct}`deco::Style` or an
{cpp:struct}`deco::AbsoluteStyle`.

The `<style>` parameter in the following signatures accepts either type.

#### Syntax

```text
deco::format_to(
    <output iterator>,
    <style>,
    <format string>,
    <args>...
);

deco::format(
    <style>,
    <format string>,
    <args>...
);
```

> ::: {doxygengroup} FormatFuncs
> :content-only:
> :::

### `deco::print` and `deco::println`

Use these functions to print formatted text with a style.

#### Syntax

```text
deco::print(
    [<FILE*> or <std::ostream&>,]
    <style>,
    <format string>,
    <args>...
);

deco::println(
    <the same parameters as deco::print>
);
```

> ::: {doxygengroup} PrintFuncs
> :content-only:
> :::


(sync_hpp)=
## Synchronized stream output

Provided by `sync.hpp`.

### OSyncStream

> :::{doxygenclass} deco::OSyncStream

(color256_hpp)=
## xterm 256-color names

Provided by `color256.hpp`. The `deco::Color256` enumeration supplies named
color indices for the xterm 256-color palette.

## Concepts

from {ref}`deco.hpp <deco_hpp>`:

> :::{doxygenconcept} deco::ostream_outputable
