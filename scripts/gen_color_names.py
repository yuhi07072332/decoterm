#!/usr/bin/env python

"""
Generates Xterm 256-color colornames by using x11 rgb.txt and meodai/colornames.
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
from collections.abc import Callable, Iterable
import argparse
import colorsys

SCRIPT_DIR: Path = Path(__file__).resolve().parent
DATA_DIR: Path = SCRIPT_DIR / "gen_color_names"

FAR_COLOR_DISTANCE: float = 0.007

class Source(Enum):
    XORG_RGB = 1
    MEODAI_COLORNAMES = 2

@dataclass(frozen=True, slots=True)
class RGB:
    r: int
    g: int
    b: int

    def __str__(self) -> str:
        brightness = 0.299 * self.r + 0.587 * self.g + 0.114 * self.b
        return  (
            f"\x1b[48;2;{self.r};{self.g};{self.b};"
            f"{"38;2;0;0;0m" if brightness > 125 else "38;2;255;255;255m"}"
            f"#{self.r:02x}{self.g:02x}{self.b:02x}"
            "\x1b[m"
        )

    def __eq__(self, other) -> bool:
        return self.r == other.r and self.g == other.g and self.b == other.b

@dataclass(frozen=True, slots=True)
class OKLab:
    l: float
    a: float
    b: float

@dataclass(frozen=True)
class ColorEntry:
    rgb: RGB
    oklab: OKLab
    name: str
    index: int | None = None

    @classmethod
    def create(cls, rgb: RGB, name: str = "", index: int | None = None):
        return cls(
            rgb=rgb,
            oklab=rgb2oklab(rgb),
            name=name,
            index=index
        )

@dataclass(frozen=True, slots=True)
class Entry:
    xterm_index: int = 0
    xterm_rgb: RGB = RGB(0, 0, 0)
    nearest_rgb: RGB | None = None
    distance: float | None = None
    name: str = ""
    source: Source | None = None
    overridden: bool = False
    rematched: bool = False

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

XORG_MUST_CONTAIN: set[str] = {
    "navy_blue",
    "dark_blue",
    "dark_green",
    "dark_cyan",
    "light_sea_green",
    "dark_turquoise",
    "medium_spring_green",
    "dark_red",
    "blue_violet",
    "steel_blue",
    "cornflower_blue",
    "cadet_blue",
    "medium_turquoise",
    "dark_magenta",
    "dark_violet",
    "purple",
    "light_slate_grey",
    "medium_purple",
    "light_slate_blue",
    "dark_sea_green",
    "light_green",
    "medium_violet_red",
    "rosy_brown",
    "dark_khaki",
    "green_yellow",
    "indian_red",
    "orchid",
    "violet",
    "tan",
    "hot_pink",
    "dark_orange",
    "light_coral",
    "sandy_brown",
}

XORG_NOT_GREYSCALE: set[str] = {
    "light_slate_grey",
    "dark_slate_grey",
    "slate_grey"
}


NAME_OVERRIDES: dict[int, str] = {
    16: "black_",
    45: "turquoise"
};

REMATCH_INDEX: set[int] = {
    23, 31, 50, 53, 54, 79, 86, 87, 122, 123, 125, 144, 145, 150, 152, 153,
    156, 157, 158, 159, 
}


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
    return [xterm_rgb(i) for i in range (0, 256)]

def load_xorg_rgb() -> list[ColorEntry]:
    xorg_rgb: list[ColorEntry] = []

    with open(DATA_DIR / "xorg-rgb" / "rgb.txt",
              mode="r",
              encoding='utf-8') as f:
        for line in f.readlines():
            r, g, b, name = line.split(maxsplit=3)
            name = colorname_to_snake_case(name.rstrip())
            rgb = RGB(int(r), int(g), int(b))

            # filter
            if name in TERMINAL_COLOR_NAMES_LOWER: continue
            if name[-1].isdigit(): continue
            if "web" in name: continue

            xorg_rgb.append(ColorEntry.create(rgb, name))

    # remove name variants with same RGB
    result: list[ColorEntry] = []
    groups: dict[RGB, list[ColorEntry]] = {}

    for entry in xorg_rgb:
        groups.setdefault(entry.rgb, []).append(entry)

    for entries in groups.values():
        selected = entries[0]
        names = {entry.name for entry in entries}

        for entry in entries:
            name = entry.name
            if "grey" in name and name.replace("grey", "gray") in names:
                selected = entry
                break

        result.append(selected)

    return result


def load_meodai_colornames(filter_good: bool) -> list[ColorEntry]:
    meodai_data : list[ColorEntry] = []

    with open(DATA_DIR / "colornames/colornames.csv",
              mode="r",
              encoding="utf-8") as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            if filter_good and row[2] != 'x': continue

            color_name: str = colorname_to_snake_case(row[0])
            if color_name in TERMINAL_COLOR_NAMES_LOWER: continue

            rgb = hex2rgb(row[1])
            meodai_data.append(ColorEntry.create(rgb, color_name))
    
    return meodai_data

def color_distance_square(lhs: OKLab, rhs: OKLab) -> float:
    return (
        (rhs.l - lhs.l) ** 2
        + (rhs.a - lhs.a) ** 2 
        + (rhs.b - lhs.b) ** 2
    )

def nearest_matches(
    input: RGB | OKLab,
    color_entries: Iterable[ColorEntry],
    count: int = 3
) -> list[tuple[ColorEntry, float]]:
    if isinstance(input, RGB): input_lab = rgb2oklab(input)
    else: input_lab = input

    return heapq.nsmallest(
        count,
        (
            (entry, color_distance_square(input_lab, entry.oklab))
            for entry in color_entries
        ),
        key=lambda item: item[1]
    )

def match_from_xorg(
    xterm_data: list[ColorEntry],
    xorg_data: list[ColorEntry],
    must_contain_name: list[str] = []
) -> dict[int, Entry]:
    """
    Returns
        dict[xterm index, matched xorg]
    """

    def match_and_add(entries: list[ColorEntry]):
        for entry in entries:
            if len(result) >= 256: break

            # Disallow matching grey entry's name to non-grey color
            if "grey" in entry.name and entry.name not in XORG_NOT_GREYSCALE: 
                data = xterm_grey
            else:
                data = xterm_nongrey

            match, distance = nearest_matches(
                entry.oklab,
                data,
                count=1)[0]

            assert match.index is not None
            assert match.index not in result

            result[match.index] = Entry(
                xterm_index=match.index,
                xterm_rgb=match.rgb,
                distance=distance,
                nearest_rgb=entry.rgb,
                name=entry.name,
                source=Source.XORG_RGB
            )
            data.remove(match)

    result: dict[int, Entry] = {}
    xterm_nongrey: set[ColorEntry] = set(xterm_data[16:232])
    xterm_grey: set[ColorEntry] = set(xterm_data[232:])

    must_contain: list[ColorEntry] = list(filter(
        lambda entry: entry.name in must_contain_name,
        xorg_data
    ))
    remaining: list[ColorEntry] = [
        entry for entry in xorg_data
        if entry.name not in must_contain_name
    ]
    
    match_and_add(must_contain)
    match_and_add(remaining)

    return result


def match_xterm_rgbs(
        xterm_data: list[ColorEntry],
        xorg_data: list[ColorEntry],
        meodai_data: list[ColorEntry]
) -> list[Entry]:
    result: list[Entry] = [Entry()] * 256
    result_names: set[str] = set()

    matched_xorg: dict[int, Entry] = match_from_xorg(
        xterm_data,
        xorg_data,
        list(XORG_MUST_CONTAIN)
    )

    for entry in matched_xorg.values():
        result_names.add(entry.name)

    for index, entry in enumerate(xterm_data):
        if index < 16: continue
        if index in matched_xorg: 
            result[index] = matched_xorg[index]
            continue

        matches = nearest_matches(entry.oklab, meodai_data, count=3)
        nearest = None
        nearest_dist = None
        for  match, distance in matches:
            if match.name in result_names: continue
            nearest = match
            nearest_dist = distance

        if nearest is None:
            print("Match error")
            sys.exit(1)

        result[index] = Entry(
            xterm_index=index,
            xterm_rgb=entry.rgb,
            nearest_rgb=nearest.rgb,
            distance=nearest_dist,
            name=nearest.name,
            source=Source.MEODAI_COLORNAMES)
        result_names.add(nearest.name)

    return result

def override_entries(entries: list[Entry]) -> list[Entry]:
    for entry in entries:
        if entry.xterm_index not in NAME_OVERRIDES: continue
        entries[entry.xterm_index] = Entry(
            xterm_rgb=entry.xterm_rgb,
            nearest_rgb=None,
            name=NAME_OVERRIDES[entry.xterm_index],
            source=None,
            overridden=True
        )

    return entries

def rematch_entries(entries: list[Entry],
                    meodai_data: list[ColorEntry]) -> list[Entry]:
    for entry in entries:
        if entry.xterm_index not in REMATCH_INDEX: continue
        matches = nearest_matches(
            entry.xterm_rgb,
            meodai_data,
            count=3
        )
        # TODO: 
    return entries
        

def check_valid(entries: list[Entry]) -> None:
    entry_names = [entry.name for entry in entries]
    for name in XORG_MUST_CONTAIN:
        if name not in entry_names:
            print(f"\x1b[91;1mERROR: must contain {name}\x1b[m", file=sys.stderr)
            sys.exit(1)

    seen: dict[str, Entry] = {}
    duplicates: dict[int, Entry] = {}

    for entry in entries:
        if entry.name in seen:
            seen_entry = seen[entry.name]
            duplicates[seen_entry.xterm_index] = seen_entry
            duplicates[entry.xterm_index] = entry
        else:
            seen[entry.name] = entry

    if len(duplicates) == 0: return
    print("\x1b[91;1mERROR: has duplicates\x1b[m", file=sys.stderr)
    print("duplicates: ", file=sys.stderr)
    for index, entry in duplicates.items():
        print(f"[{index}] {entry.xterm_rgb}: {entry.name} {entry.source}")
    sys.exit(1)

def preview_loaded(entries: list[ColorEntry]) -> None:
    for entry in entries:
        print(f"{entry.rgb}: {entry.name}")

def preview(
        entries: list[Entry],
    ) -> None:
    print("\x1b[93mYellow color names are from meodai/colornames !\x1b[m\n")
    for entry in entries:
        print(f"[{entry.xterm_index:0>3}] ", end='')
        print(f"{entry.xterm_rgb}", end='')

        if entry.nearest_rgb is not None:
            if entry.nearest_rgb == entry.xterm_rgb:
                print("     (same)", end='')
            else:
                print(f" <- {entry.nearest_rgb}", end='')
        else: 
            print("           ", end='')
        print(" : ", end='')

        if (entry.source is not None
            and entry.source == Source.MEODAI_COLORNAMES):
            print("\x1b[33m", end='')
        print(f"{entry.name:<23}\x1b[m", end='')

        if entry.nearest_rgb is not None:
            print("  dist:", end='')
            if (entry.distance is not None
                and entry.distance > FAR_COLOR_DISTANCE):
                print("\x1b[91;1m", end='')
            print(f"{entry.distance:<.6f}\x1b[m", end='')

        if entry.overridden:
            print(" \x1b[91;1m(overridden)\x1b[m", end='')
        elif entry.rematched:
            print(" \x1b[96;1m(rematched)\x1b[m", end='')
        print()

    print(f"Entry count: {len(entries)}")


def generate_info(xterm_rgbs) -> None:
    print("inline constexpr std::array<ColorInfo, 256> color_info {{")
    for rgb in xterm_rgbs:
        print(f"    {{ {rgb.r}, {rgb.g}, {rgb.b} }},")
    print("}};")
        

def generate_enum(result: list[Entry]) -> None:
    print("enum TerminalColors: uint8_t {")
    for i, name in enumerate(TERMINAL_COLOR_NAMES):
        print(f"    {pascalcase_to_snake_case(name):<25} = {i:<3},    // {TERMINAL_COLOR_RGBS[i]}")

    print()

    for i, entry in enumerate(result, 16):
        print(f"    {entry.name:<25} = {i:<3},    // {entry.xterm_rgb}")

    print("};")

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()


    subparsers = parser.add_subparsers(
        dest="command",
        required=True,
    )

    # preview
    preview_parser = subparsers.add_parser(
        "preview",
        help="Preview generated/loaded entries",
    )
    preview_parser.add_argument(
        "target",
        nargs="?",
        choices=[
            "full",
            "xorg",
            "meodai",
            "loaded_xorg",
            "loaded_meodai",
        ],
        default="full",
        help="Preview target (default: full)",
    )

    # generate
    generate_parser = subparsers.add_parser(
        "generate",
        help="Generate source data",
    )
    generate_parser.add_argument(
        "target",
        choices=[
            "enum",
            "info",
        ],
        help="Generate target",
    )

    subparsers.add_parser(
        "debug"
    )

    return parser.parse_args()

def main():
    args = parse_args()

    xterm_rgbs: list[RGB] = compute_xterm_rgb()
    assert len(xterm_rgbs) == 256

    if args.command == "generate" and args.target == "info":
        generate_info(xterm_rgbs)
        return;

    xterm_data: list[ColorEntry] = [
        ColorEntry.create(rgb, index=i) for i, rgb in enumerate(xterm_rgbs)
    ]
    xorg_data: list[ColorEntry] = load_xorg_rgb()
    meodai_data: list[ColorEntry] = load_meodai_colornames(True)

    if args.command == "preview":
        match args.target:
            case "loaded_xorg": 
                preview_loaded(xorg_data)
                return
            case "loaded_meodai":
                preview_loaded(meodai_data)
                return

    result: list[Entry] = match_xterm_rgbs(
        xterm_data,
        xorg_data,
        meodai_data,
    )

    for entry in result:
        if (entry.distance is not None
            and entry.distance > FAR_COLOR_DISTANCE):
            REMATCH_INDEX.add(entry.xterm_index)
    print(list(REMATCH_INDEX))

    if args.command != "debug":
        override_entries(rematch_entries(result, meodai_data))

    check_valid(result[16:])

    if args.command == "preview":
        match args.target:
            case "full": preview(result[16:])
            case "xorg": 
                preview([
                    entry for entry in result[16:]
                    if entry.source == Source.XORG_RGB
                ])
            case "meodai":
                preview([
                    entry for entry in result[16:]
                    if entry.source == Source.MEODAI_COLORNAMES
                ])
    elif args.command == "generate" and args.target == "enum":
        generate_enum(result[16:])
    elif args.command == "debug":
        preview(result[16:])
        pass

if __name__ == "__main__":
    main()

