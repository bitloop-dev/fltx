#!/usr/bin/env python3
"""Run the complete fltx CI checks from one CMake build preset."""

from __future__ import annotations

import sys
from pathlib import Path


VALIDATION_DIR = Path(__file__).resolve().parent
ROOT = VALIDATION_DIR.parent
sys.path.insert(0, str(VALIDATION_DIR / "_internal"))

from check_pipeline import main  # noqa: E402


if __name__ == "__main__":
    raise SystemExit(main(ROOT))
