#!/usr/bin/env python3
"""Configure one CMake preset and run the complete fltx CI check target."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import Mapping, Sequence

from preset_support import (
    PipelineError,
    PresetSelection,
    msvc_environment,
    msvc_target_architecture,
    needs_msvc_environment,
    resolve_preset,
)


CHECK_TARGET = "fltx_ci_checks"


def _run(
    command: Sequence[str],
    root: Path,
    environment: Mapping[str, str] | None = None,
) -> None:
    print(f"> {subprocess.list2cmdline(list(command))}", flush=True)
    try:
        result = subprocess.run(
            command,
            cwd=root,
            env=dict(environment) if environment is not None else None,
            check=False,
        )
    except OSError as error:
        raise PipelineError(f"cannot run {command[0]}: {error}") from error
    if result.returncode:
        raise PipelineError(
            f"command exited with status {result.returncode}: "
            f"{subprocess.list2cmdline(list(command))}"
        )


def _build_environment(selection: PresetSelection) -> Mapping[str, str]:
    if not needs_msvc_environment(selection):
        return selection.environment
    return msvc_environment(
        msvc_target_architecture(selection),
        parent_environment=selection.environment,
    )


def run_checks(root: Path, preset: str) -> None:
    root = root.resolve()
    selection = resolve_preset(root, preset)
    environment = _build_environment(selection)

    try:
        _run(
            ["cmake", "--preset", selection.configure_name],
            root,
            environment,
        )
    except PipelineError as error:
        hints = f"{selection.compiler_hint} {selection.target_hint}".casefold()
        if "mingw" not in hints:
            raise
        raise PipelineError(
            f"{error}\nMinGW preset {selection.configure_name!r} requires "
            f"a MinGW compiler and {selection.generator} in the invoking "
            "environment or a local CMake user preset; the vcpkg triplet "
            "selects dependency binaries but does not provide these tools."
        ) from error

    _run(
        [
            "cmake",
            "--build",
            "--preset",
            selection.build_name,
            "--target",
            CHECK_TARGET,
            "--parallel",
        ],
        root,
        environment,
    )


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", required=True, help="CMake build preset name")
    return parser.parse_args(argv)


def main(root: Path, argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        run_checks(root, args.preset)
    except PipelineError as error:
        print(f"preset checks failed: {error}", file=sys.stderr)
        return 1
    print(f"preset checks passed: {args.preset}", flush=True)
    return 0
