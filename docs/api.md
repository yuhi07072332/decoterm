---
tocdepth: 3
---

# API Reference

(header_overview)=
## Header Overview

**Dependency Chart**

:::{md-mermaid}
flowchart TD
    deco[deco.hpp] --> output[output.hpp]
    output --> sync[sync.hpp]
    output --> format[format.hpp]
    output --> terminal[terminal.hpp]
    color256[color256.hpp]
:::

- {ref}`deco.hpp <deco_hpp>`: Defines core types such as 
{cpp:struct}`deco::Color` and {cpp:struct}`deco::Style`, which can be written
directly to {cpp:class}`std::ostream` or used with {cpp:func}`std::print`. 
- {ref}`output.hpp <output_hpp>`: Provides advanced style output functionality 
such as style output configuration and styled writers.
- {ref}`format.hpp <format_hpp>`: Provides some wrapper functions for 
{cpp:func}`std::format` and {cpp:func}`std::print`.
- {ref}`terminal.hpp <terminal_hpp>`: Provides terminal utilities.
- {ref}`sync.hpp <sync_hpp>`: Provides {cpp:class}`deco::OSyncStream`.
- {ref}`color256.hpp <color256_hpp>`: Color names for Xterm 256 colors.

## Configuration Macros

- `DECOTERM_NO_FORMAT`: Disables `<format>` and `<print>` functionality.
You can define this before including any decoterm header file to improve the
compile time.

(deco_hpp)=
## Core

Provided from `deco.hpp`.

### Color

> :::{doxygenstruct} deco::Color

> :::{doxygenenum} deco::ColorType

### Colors

There are some helper functions and pre-defined constants in 
`inline namespace colors`.

::: {tip}
You can use `using namespace deco::colors` to avoid verbosity.
:::

Helper functions for creating `Color`:

> ::: {doxygengroup} ColorHelpers
> :content-only:
> :::

Pre-defined `Color` constants:

> ::: {doxygengroup} ColorConstants
> :content-only:
> :::

### Style

> :::{doxygenstruct} deco::Style

> :::{doxygenstruct} deco::AbsoluteStyle

### Styles

There are some helper functions and pre-defined constants in 
`inline namespace styles`.

::: {tip}
You can use `using namespace deco::styles` to avoid verbosity.
:::

Helper functions for creating `Style`:

> ::: {doxygengroup} StyleHelpers
> :content-only:
> :::

Pre-defined `Style` constants:

> ::: {doxygengroup} StyleConstants
> :content-only:
> :::


### Styled Value

Similar to [{fmt}](https://fmt.dev), You can wrap a value with a style by using
{cpp:func}`styled`.

> :::{doxygenfunction} styled(const T&, Style)
> :::
> :::{doxygenfunction} styled(const T&, AbsoluteStyle)

You can also use `value | style` for shorthand.

> :::{doxygenfunction} operator|(const T&, Style)
> :::
> :::{doxygenfunction} operator|(const T&, AbsoluteStyle)

(output_hpp)=
## Advanced Style Output

Provided from `output.hpp`.

### Output Configuration

You can apply `deco::OutputConfig` to {ref}`Styled Writers<api_styled_writers>`.

> :::{doxygenclass} deco::OutputConfig

You can create a custom config, or use the **global config**.

> :::{doxygenfunction} deco::config

(api_styled_writers)=
### Styled Writers

There are 2 styled writers: {cpp:class}`deco::OStream` and {cpp:class}`deco::Printer`.

> :::{doxygenclass} deco::OStream

> :::{doxygenfunction} deco::stdout_ostream

> :::{doxygenfunction} deco::stderr_ostream

> :::{doxygenclass} deco::Printer

> :::{doxygenfunction} deco::stdout_printer

> :::{doxygenfunction} deco::stderr_printer

(terminal_hpp)=
## Terminal Utilities

Provided from `terminal.hpp`.

### Fetching Terminal Information

> :::{doxygenstruct} deco::TerminalInfo

> :::{doxygenfunction} deco::terminal_info

### Initializing the output

> :::{doxygenfunction} deco::init

(format_hpp)=
## `format` / `print` Wrappers

Provided from `format.hpp`.

### format

You can specify a style for formatting.

**Syntax**

```text
deco::format_to(
    <output iterator>,
    <Style or AbsoluteStyle,>
    <format string>,
    <args>...
);

deco::format(
    <Style or AbsoluteStyle,>
    <format string>,
    <args>...
);
```

> ::: {doxygengroup} FormatFuncs
> :content-only:
> :::


### print

You can specify a style for printing.

**Syntax**

```text
deco::print(
    [FILE* or std::ostream&,]
    <Style or AbsoluteStyle,>
    <format string>,
    <args>...
);

deco::println(
    <same as deco::print>
);
```

> ::: {doxygengroup} PrintFuncs
> :content-only:
> :::


(sync_hpp)=
## Sync

Provided from `sync.hpp`.

### OSyncStream

> :::{doxygenclass} deco::OSyncStream

(color256_hpp)=
## 256 Color Names

Provided from `color256.hpp`.


## Concepts

> :::{doxygenconcept} deco::ostream_outputable
