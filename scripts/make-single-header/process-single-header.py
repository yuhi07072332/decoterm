#!/usr/bin/env python

"""
Process auto generated single-include/decoterm.hpp header.
removes:
    commented out '// #include' lines
    License information comments
"""

from pathlib import Path
import sys
import logging as log

SCRIPT_DIR: Path = Path(__file__).resolve().parent
target_path: Path = Path(sys.argv[1]).resolve()

def process(target_path: Path):
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
