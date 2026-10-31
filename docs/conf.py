# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "decoterm"
copyright = "2026, Yuhi0707"
author = "Yuhi0707"

version = "0.3.0"
release = "0.3.0"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "breathe",
    "myst_parser",
    "sphinx_immaterial",
    # "sphinx_immaterial.apidoc.format_signatures",
    "sphinx_immaterial.apidoc.cpp.cppreference",
]

exclude_patterns = []

myst_enable_extensions = ["colon_fence"]

master_doc = "index"


# Sphinx-immaterial config

html_theme_options = {
    "repo_url": "https://github.com/yuhi07072332/decoterm",
    "icon": {
        "repo": "fontawesome/brands/github",
    },
    "features": [
        "content.code.copy"
        "search.suggest",
        "search.highlight",
        "navigation.sections",
        "navigation.top",
        "navigation.tabs",
        "navigation.sticky",
    ],
    "palette": [
        {
            "media": "(prefers-color-scheme)",
            "scheme": "default",
            "toggle": {
                "icon": "material/weather-night",
                "name": "Switch to light mode",
            }
        },
        {
            "media": "(prefers-color-scheme: light)",
            "scheme": "default",
            "toggle": {
                "icon": "material/weather-sunny",
                "name": "Switch to dark mode",
            }
        },
        {
            "media": "(prefers-color-scheme: dark)",
            "scheme": "slate",
            "toggle": {
                "icon": "material/lightbulb",
                "name": "Switch to system preference",
            }
        },
    ],
}

object_description_options = [
    ("cpp:.*Param", dict(include_in_toc=False)),
    ("cpp:.*", dict(
        include_fields_in_toc=False,
        # clang_format_style=CLANG_FORMAT_STYLE,
    )),
]

# Breathe config
breathe_default_project = "decoterm"
breathe_default_members = ( "members", "undoc-members" )


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_title = f"decoterm {release}"

html_theme = "sphinx_immaterial"
