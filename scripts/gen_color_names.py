#!/usr/bin/env python

"""
Generates Xterm 256-color colornames by using meodai/colornames.
Usage:
    gen_color_names.py <preview/generate-enum/generate-info>
"""

import sys
import csv
import xml.etree.ElementTree as ET
import heapq
from dataclasses import dataclass
from pathlib import Path
from enum import Enum
from math import cbrt

SCRIPT_DIR: Path = Path(__file__).resolve().parent
DATA_DIR: Path = SCRIPT_DIR / "gen_color_names"

TERMINAL_COLOR_NAMES: list[str] = [
    "Black",
    "Red",
    "Green",
    "Yellow",
    "Blue",
    "Magenta",
    "Cyan",
    "White",
    "BrightBlack",
    "BrightRed",
    "BrightGreen",
    "BrightYellow",
    "BrightBlue",
    "BrightMagenta",
    "BrightCyan",
    "BrightWhite",
]

TERMINAL_COLOR_NAMES_LOWER: list[str] = [
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "magenta",
    "cyan",
    "white",
]

class Command(Enum):
    PREVIEW = 1
    GENERATE_ENUM = 2
    GENERATE_INFO = 3

class Source(Enum):
    NONE = 0
    XORG_RGB = 1
    MEODAI_COLORNAMES = 2

@dataclass(frozen=True, slots=True)
class RGB:
    r: int
    g: int
    b: int

    def __str__(self) -> str:
        return f"#{self.r:02x}{self.g:02x}{self.b:02x}"

    def __eq__(self, other) -> bool:
        return self.r == other.r and self.g == other.g and self.b == other.b

@dataclass(frozen=True, slots=True)
class OKLab:
    l: float
    a: float
    b: float

@dataclass
class ColorEntry:
    rgb: RGB
    oklab: OKLab
    name: str

@dataclass(frozen=True, slots=True)
class Entry:
    xterm_rgb: RGB = RGB(0, 0, 0)
    nearest_rgb: RGB = RGB(0, 0, 0)
    name: str = ""
    source: Source = Source.NONE

TERMINAL_COLOR_RGBS: list[RGB] = [
    RGB( 0, 0, 0 ),
    RGB( 128, 0, 0 ),
    RGB( 0, 128, 0 ),
    RGB( 128, 128, 0 ),
    RGB( 0, 0, 128 ),
    RGB( 128, 0, 128 ),
    RGB( 0, 128, 128 ),
    RGB( 192, 192, 192 ),
    RGB( 128, 128, 128 ),
    RGB( 255, 0, 0 ),
    RGB( 0, 255, 0 ),
    RGB( 255, 255, 0 ),
    RGB( 0, 0, 255 ),
    RGB( 255, 0, 255 ),
    RGB( 0, 255, 255 ),
    RGB( 255, 255, 255 )
]

def hex2rgb(hex_str: str) -> RGB:
    hex_str = hex_str.lstrip('#')
    return RGB(int(hex_str[0 : 2], 16), int(hex_str[2 : 4], 16), int(hex_str[4 : 6], 16))

def srgb2linear(x: float) -> float:
    if x <= 0.04045:
        return x / 12.92
    return ((x + 0.055) / 1.055) ** 2.4

def rgb2oklab(rgb: RGB) -> OKLab:
    r = srgb2linear(rgb.r / 255.0)
    g = srgb2linear(rgb.g / 255.0)
    b = srgb2linear(rgb.b / 255.0)

    l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b
    m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b
    s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b

    l_ = cbrt(l)
    m_ = cbrt(m)
    s_ = cbrt(s)

    return OKLab(
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_,
    )

# see https://web.archive.org/web/20130125000058/http://www.frexx.de/xterm-256-notes/

def xterm_rgb(index: int) -> RGB:
    if index < 16:
        return TERMINAL_COLOR_RGBS[index]

    if 16 <= index <= 231:
        i = index - 16

        r = i // 36
        g = (i % 36) // 6
        b = i % 6

        levels = [0, 95, 135, 175, 215, 255]

        return RGB(
            levels[r],
            levels[g],
            levels[b]
        )
    
    if 232 <= index <= 255:
        gray = 8 + (index - 232) * 10
        return RGB(gray, gray, gray)

    raise ValueError("")


def compute_xterm_rgb() -> list[RGB]:
    return [xterm_rgb(i) for i in range (0, 255)]

def load_xorg_rgb() -> list[ColorEntry]:
    xorg_rgb: list[ColorEntry] = []

    with open(DATA_DIR / "xorg-rgb" / "rgb.txt",
              mode="r",
              encoding='utf-8') as f:
        for line in f.readlines():
            r, g, b, name = line.split(maxsplit=3)
            name = name.rstrip()
            rgb = RGB(int(r), int(g), int(b))

            if name in map(lambda name: name.lower(), TERMINAL_COLOR_NAMES_LOWER):
                continue
            if name[len(name) - 1].isdigit(): continue
            if next((entry for entry in xorg_rgb
                if entry.rgb == rgb), None) is not None: continue

            xorg_rgb.append(ColorEntry(
                rgb,
                rgb2oklab(rgb),
                name
            ))

    return xorg_rgb


def load_meodai_colornames(filter_good: bool) -> list[ColorEntry]:
    meodai_data : list[ColorEntry] = []

    with open(DATA_DIR / "colornames/colornames.csv",
              mode="r",
              encoding="utf-8") as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            if row[2] != 'x': continue
            color_name: str = row[0]
            rgb = hex2rgb(row[1])
            meodai_data.append(ColorEntry(
                rgb,
                rgb2oklab(rgb),
                color_name
            ))
    
    return meodai_data

def color_distance_square(lhs: OKLab, rhs: OKLab) -> float:
    return (
        (rhs.l - lhs.l) ** 2
        + (rhs.a - lhs.a) ** 2 
        + (rhs.b - lhs.b) ** 2
    )

def nearest_matches(
    input: RGB,
    color_entries: list[ColorEntry],
    excluded_names: set[str],
    count: int = 3
) -> list[tuple[RGB, str, float]]:
    input_lab = rgb2oklab(input)

    matches = heapq.nsmallest(
        count,
        (
            (color_distance_square(input_lab, entry.oklab), entry)
            for entry in color_entries
            if entry.name not in excluded_names
        ),
        key=lambda item: item[0],
    )

    return [
        (entry.rgb, entry.name, distance)
        for distance, entry in matches
    ]

    return nearest.rgb, nearest.name, distance

def match_xterm_rgbs(xterm_rgbs: list[RGB],
          xorg_data: list[ColorEntry],
          meodai_data: list[ColorEntry]) -> list[Entry]:
    term_color_names = set(TERMINAL_COLOR_NAMES)
    term_color_names_lower = set(TERMINAL_COLOR_NAMES_LOWER)

    result: list[Entry] = []
    result_names: list[str] = []

    for rgb in xterm_rgbs:
        nearest = nearest_matches(rgb, xorg_data, term_color_names_lower, 1)[0]
        if not nearest[1] in result_names:
            result.append(Entry(rgb, nearest[0], nearest[1], Source.XORG_RGB))
            result_names.append(nearest[1])
            continue

        matches = [match for match in 
                   nearest_matches(rgb, meodai_data, term_color_names, 3)
                   if match[1] not in result_names]

        nearest = min(matches, key=lambda item: item[2])
        result.append(Entry(rgb, nearest[0], nearest[1], Source.MEODAI_COLORNAMES))
        result_names.append(nearest[1])

    return result


def pascalcase_to_snake_case(input: str) -> str:
    result: str = ""
    begin: bool = True
    for i, c in enumerate(input):
        if c.isupper():
            if i != 0: result += '_'
            result += c.lower()
        else:
            result += c

    return result

def colorname_to_snake_case(input: str) -> str:
    result: str = ""
    for c in input:
        if c == '’' or c == '!':
            continue

        if c == '-' or c == ' ':
            result += '_'
            continue

        result += c.lower()
        
    return result

def generate_info(xterm_rgbs) -> None:
    print("inline constexpr std::array<ColorInfo, 256> color_info {{")
    for rgb in xterm_rgbs:
        print(f"    {{ {rgb.r}, {rgb.g}, {rgb.b} }},")
    print("}};")
        

def generate_enum(result: list[Entry]) -> None:
    print("enum TerminalColors: uint8_t {")
    for i, name in enumerate(TERMINAL_COLOR_NAMES_LOWER):
        print(f"    {pascalcase_to_snake_case(name):<25} = {i:<3},    // {TERMINAL_COLOR_RGBS[i]}")

    print()

    for i, entry in enumerate(result, 16):
        name = colorname_to_snake_case(entry.name)
        print(f"    {name:<25} = {i:<3},    // {entry.xterm_rgb}")

    print("};")


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} [preview/generate-enum/generate-info]")
        sys.exit(1)

    match sys.argv[1]:
        case "preview": command = Command.PREVIEW
        case "generate-enum": command = Command.GENERATE_ENUM
        case "generate-info": command = Command.GENERATE_INFO
        case _:
            print(f"Unknown command: {sys.argv[1]}")
            sys.exit(1)

    xterm_rgbs = compute_xterm_rgb()
    assert len(xterm_rgbs) == 255

    if command == Command.GENERATE_INFO:
        generate_info(xterm_rgbs)
        return;

    xorg_data = load_xorg_rgb()
    meodai_data = load_meodai_colornames(True)

    result: list[Entry] = match_xterm_rgbs(
        xterm_rgbs[16:],
        xorg_data,
        meodai_data
    )

    print("Checking duplicates...", end='')
    result_names: list[str] = [entry.name for entry in result]
    if (len(result_names) != len(set(result_names))):
        print("\n\x1b[31;1mERROR: has duplicates\x1b[m")
        sys.exit(1)
    print("  \x1b[32mOK\x1b[m")

    if command == Command.PREVIEW:
        for i, entry in enumerate(result, start=16):
            print(f"[{i:0>3}] ", end='')
            if entry.source == Source.XORG_RGB:
                print(f"{entry.xterm_rgb} <- {entry.nearest_rgb}: {entry.name}")
            elif entry.source == Source.MEODAI_COLORNAMES:
                print(f"{entry.xterm_rgb} <- {entry.nearest_rgb}: \x1b[33m{entry.name}\x1b[m")
    elif command == Command.GENERATE_ENUM: 
        generate_enum(result)

if __name__ == "__main__":
    main()

