#!/usr/bin/env python

from pathlib import Path
import subprocess
import sys

PROJECT_ROOT = Path(__file__).resolve().parent.parent
DOCS = PROJECT_ROOT / "docs"
BUILD = PROJECT_ROOT / "build" / "docs"

DOXYGEN_XML = BUILD / "doxygen" / "xml"
HTML = BUILD / "html"

def main():
    BUILD.mkdir(parents=True, exist_ok=True)

    print("building documents...")

    # Generate Doxygen XML
    subprocess.run(
        ["doxygen", str(DOCS / "Doxyfile")],
        cwd=PROJECT_ROOT,
        check=True,
    )

    # Generate Sphinx HTML
    subprocess.run(
        [
            sys.executable, "-m", "sphinx",
            "-b", "html",
            "-D", f"breathe_projects.decoterm={DOXYGEN_XML}",
            str(DOCS),
            str(HTML),
        ],
        cwd=PROJECT_ROOT,
        check=True,
    )

if __name__ == "__main__":
    main()
