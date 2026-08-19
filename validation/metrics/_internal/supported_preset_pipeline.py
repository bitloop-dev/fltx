#!/usr/bin/env python3
"""Run metrics for every supported preset on the current host."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

VALIDATION_INTERNAL = Path(__file__).resolve().parents[2] / "_internal"
sys.path.insert(0, str(VALIDATION_INTERNAL))

import preset_pipeline
from preset_support import (
    SUPPORTED_PRESETS,
    WASM_PRESET,
    supported_presets as _supported_presets,
)


def supported_presets(
    host_system: str | None = None,
    host_machine: str | None = None,
) -> tuple[str, ...]:
    return _supported_presets(host_system, host_machine)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    preset_pipeline.add_workflow_arguments(parser)
    return parser.parse_args(argv)


def main(root: Path, argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        presets = supported_presets()
    except preset_pipeline.PipelineError as error:
        print(f"all-supported metrics failed: {error}", file=sys.stderr)
        return 1

    for index, preset in enumerate(presets, start=1):
        print(
            f"running metrics preset {index}/{len(presets)}: {preset}",
            flush=True,
        )
        try:
            outputs = preset_pipeline.run_pipeline_from_args(root, preset, args)
        except preset_pipeline.PipelineError as error:
            print(
                f"all-supported metrics failed for {preset}: {error}",
                file=sys.stderr,
            )
            return 1
        print(f"generated metrics reports for {preset}:", flush=True)
        for output in outputs:
            print(output, flush=True)
    return 0
