#!/usr/bin/env python3
"""Build a local HTML viewer for the existing non-compact overview SVGs."""

from __future__ import annotations

import sys
from pathlib import Path


METRICS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(METRICS_DIR / "_internal"))

from overview_viewer import main  # noqa: E402


if __name__ == "__main__":
    raise SystemExit(main(METRICS_DIR))
