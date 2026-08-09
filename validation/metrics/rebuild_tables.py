#!/usr/bin/env python3
"""Rebuild all metrics tables and overviews from existing data."""

from __future__ import annotations

import sys
from pathlib import Path


METRICS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(METRICS_DIR / "_internal"))

from report_pipeline import main  # noqa: E402


if __name__ == "__main__":
    raise SystemExit(main(METRICS_DIR))
