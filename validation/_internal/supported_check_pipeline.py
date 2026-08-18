#!/usr/bin/env python3
"""Run the complete fltx CI check target for every supported host preset."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import check_pipeline
from preset_support import PipelineError, supported_presets


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    return parser.parse_args(argv)


def main(root: Path, argv: list[str] | None = None) -> int:
    parse_args(argv)
    try:
        presets = supported_presets()
    except PipelineError as error:
        print(f"all-supported checks failed: {error}", file=sys.stderr)
        return 1

    for index, preset in enumerate(presets, start=1):
        print(
            f"running check preset {index}/{len(presets)}: {preset}",
            flush=True,
        )
        try:
            check_pipeline.run_checks(root, preset)
        except PipelineError as error:
            print(
                f"all-supported checks failed for {preset}: {error}",
                file=sys.stderr,
            )
            return 1
        print(f"preset checks passed: {preset}", flush=True)
    return 0
