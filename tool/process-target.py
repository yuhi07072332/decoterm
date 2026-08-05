#!/usr/bin/env python

"""
Process auto generated single-include/decoterm.hpp header.
Usage: process-target.py <path/to/target>
removes:
    commented out '// #include' lines
    License information comments
"""

from pathlib import Path
import sys
import logging as log

if len(sys.argv) < 2:
    sys.stderr.write(f"Usage: {sys.argv[0]} <path/to/target> [--debug]\n")
    sys.exit(1)

debug: bool = False
if len(sys.argv) >= 3 and sys.argv[2] == "--debug":
    debug = True

log.basicConfig(
    level = log.DEBUG if debug else log.INFO,
)

SCRIPT_DIR: Path = Path(__file__).resolve().parent
TARGET_PATH: Path = Path(sys.argv[1]).resolve()
log.info(f"found target: {TARGET_PATH}")

with open(TARGET_PATH, "r", encoding="utf-8") as target_file:
    target_lines = target_file.readlines()


with open(TARGET_PATH, "w", encoding="utf-8") as target_file:
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
