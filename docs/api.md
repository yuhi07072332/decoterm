---
tocdepth: 3
---

# API Reference

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

- {ref}`deco.hpp <core_api>`: The core/minimal API. It defines core types such 
  as {cpp:struct}`deco::Color` and {cpp:struct}`deco::Style`, which can be written
  directly to `std::ostream` or used with `std::print`.

- {ref}`output.hpp <styled_output_api>`: 

(core_api)=
## Core API

### Color

> :::{doxygenenum} deco::ColorType

> :::{doxygenstruct} deco::Color

### Colors

> :::{doxygennamespace} deco::colors

### Style

> :::{doxygenstruct} deco::Style

> :::{doxygenstruct} deco::AbsoluteStyle

### Styles

> :::{doxygennamespace} deco::styles

### Styled Value

Similar to [{fmt}](https://fmt.dev), You can wrap a value with a style by using
{cpp:func}`styled`.

> :::{doxygenfunction} styled(const T&, Style)
> :::
> :::{doxygenfunction} styled(const T&, AbsoluteStyle)
> :::

You can also use `value | style` for shorthand.

> :::{doxygenfunction} operator|(const T&, Style)
> :::
> :::{doxygenfunction} operator|(const T&, AbsoluteStyle)

(styled_output_api)=
## Styled Output API

### Output Configuration

> :::{doxygenclass} deco::OutputConfig

> :::{doxygenfunction} deco::config

### Styled Writers

> :::{doxygenclass} deco::OStream

> :::{doxygenclass} deco::Printer

## Concepts

> :::{doxygenconcept} deco::ostream_outputable
