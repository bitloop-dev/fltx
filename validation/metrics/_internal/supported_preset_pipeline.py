#!/usr/bin/env python3
"""Run metrics for every supported preset on the current host."""

from __future__ import annotations

import argparse
import platform
import sys
from pathlib import Path

import preset_pipeline


WASM_PRESET = "wasm32-emscripten-release"
SUPPORTED_PRESETS = {
    ("Windows", "x86_64"): (
        "windows-x64-msvc-release",
        "windows-x64-clangcl-release",
        "windows-x64-mingw-release",
        WASM_PRESET,
    ),
    ("Windows", "arm64"): (
        "windows-arm64-msvc-release",
        "windows-arm64-clangcl-release",
    ),
    ("Linux", "x86_64"): (
        "linux-x64-gcc-release",
        "linux-x64-clang-release",
    ),
    ("Linux", "arm64"): (
        "linux-arm64-gcc-release",
        "linux-arm64-clang-release",
    ),
    ("Darwin", "x86_64"): (
        "macos-x64-appleclang-release",
    ),
    ("Darwin", "arm64"): (
        "macos-arm64-appleclang-release",
    ),
}
_ARCHITECTURE_ALIASES = {
    "amd64": "x86_64",
    "x64": "x86_64",
    "x86_64": "x86_64",
    "aarch64": "arm64",
    "arm64": "arm64",
}


def supported_presets(
    host_system: str | None = None,
    host_machine: str | None = None,
) -> tuple[str, ...]:
    system = host_system or platform.system()
    machine = host_machine or platform.machine()
    architecture = _ARCHITECTURE_ALIASES.get(machine.casefold(), machine.casefold())
    presets = SUPPORTED_PRESETS.get((system, architecture))
    if presets is None:
        raise preset_pipeline.PipelineError(
            f"unsupported metrics host {system}/{machine}"
        )
    return presets


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
