# Usage

## Style

### Defining Colors

There are a few ways to define a color.

:::{literalinclude} ../examples/usage-style.cpp
:language: cpp
:start-after: // [defining-color]
:end-before: // [/defining-color]
:dedent: 4
:::

:::{seealso}
{ref}`colors <api_colors>` for more information.
:::

### Defining Styles

:::{literalinclude} ../examples/usage-style.cpp
:language: cpp
:start-after: // [defining-style]
:end-before: // [/defining-style]
:dedent: 4
:::

For more information, see {ref}``. 

:::{seealso}
{ref}`styles <api_styles>` for more information.
:::

### Output Styles

{cpp:struct}`deco::Style` supports both `std::ostream` output and 
`std::format` / `std::print` output.  
It will generate the **ANSI escape sequence** to change the terminal style
attributes.

You can also use {cpp:var}`deco::reset` to reset the style.

:::{literalinclude} ../examples/usage-style.cpp
:language: cpp
:start-after: // [output-style]
:end-before: // [/output-style]
:dedent: 4
:::

### Wrapping value with a Style

:::{literalinclude} ../examples/usage-style.cpp
:language: cpp
:start-after: // [styled]
:end-before: // [/styled]
:dedent: 4
:::

:::{seealso}
{ref}`Styled values<api_styled_values>` for more information.
:::
