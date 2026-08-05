#!/usr/bin/env python

from pathlib import Path
import sys
import json
import logging as log
import runpy

"""
Generate single-include/decoterm.hpp header.
"""

SCRIPT_DIR: Path = Path(__file__).resolve().parent
CONFIG_DIR: Path = SCRIPT_DIR / "make-single-header"
TARGET_PATH: Path = SCRIPT_DIR.parent / "single-include" / "decoterm" / "decoterm.hpp"
AMALGAMATE_PATH: Path = CONFIG_DIR / "amalgamate" / "amalgamate.py"

def generate_config(config_path: Path) -> None:
    """
    Generate make-single-header/config.json
    """
    config = {
        "target": str(TARGET_PATH),
        "sources": [
            "decoterm/decoterm.hpp"
        ],
        "include_paths": ["decoterm"]
    }

    with open(config_path, "w", encoding="UTF-8") as config_file:
        json.dump(config, config_file)

def process(target_path: Path) -> None:
    """
    Process auto generated single-include/decoterm.hpp header.
    removes:
        commented out '// #include' lines
        License information comments
    """

    log.info(f"found target: {target_path}")

    with open(target_path, "r", encoding="utf-8") as target_file:
        target_lines = target_file.readlines()


    with open(target_path, "w", encoding="utf-8") as target_file:
        skip: int = 0
        for linenum, line in enumerate(target_lines):
            if skip > 0: 
                skip -= 1
                continue
            if line.startswith("// #include") : 
                log.debug(f"[Remove] {linenum}: {line.rstrip()}")
                if (target_lines[linenum + 1] == "\n"):
                    log.debug(f"[Remove] {linenum + 1}: ")
                    skip = 1
                continue

            if (line.startswith("// SPDX-License-Identifier")
                and target_lines[linenum + 1].startswith("// Copyright (c) 2026")
                    and target_lines[linenum + 2].startswith("// This file is part of the decoterm library.")):
                log.debug(f"[Remove] {linenum}: {line.rstrip()}")
                log.debug(f"[Remove] {linenum + 1}: {target_lines[linenum + 1].rstrip()}")
                log.debug(f"[Remove] {linenum + 2}: {target_lines[linenum + 2].rstrip()}")
                log.debug(f"[Remove] {linenum + 3}: {target_lines[linenum + 3].rstrip()}")
                log.debug("")
                skip = 3
                continue

            target_file.write(line)

def main():
    debug: bool = False
    if len(sys.argv) >= 2 and sys.argv[1] == "--debug":
        debug = True

    log.basicConfig(
        level = log.DEBUG if debug else log.INFO,
    )

    generate_config(CONFIG_DIR / "config.json")

    sys.argv = ["amalgamate.py",
                "-s", str(SCRIPT_DIR.parent / "include"),
                "-c", str(CONFIG_DIR / "config.json"),
                "-p", str(CONFIG_DIR / "prologue.hpp")]
    runpy.run_path(str(AMALGAMATE_PATH), run_name = "__main__")
    process(TARGET_PATH)

if __name__ == "__main__":
    main()
