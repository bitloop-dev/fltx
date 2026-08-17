#!/usr/bin/env python3
"""Run metrics for every supported preset on the current host."""

from __future__ import annotations

import sys
from pathlib import Path


METRICS_DIR = Path(__file__).resolve().parent
ROOT = METRICS_DIR.parents[1]
sys.path.insert(0, str(METRICS_DIR / "_internal"))

from supported_preset_pipeline import main  # noqa: E402


if __name__ == "__main__":
    raise SystemExit(main(ROOT))
