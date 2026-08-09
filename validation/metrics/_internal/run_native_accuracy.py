#!/usr/bin/env python3
"""Run the complete f32/f64 accuracy suites without publishing summary tables."""

from __future__ import annotations

import argparse
import sys
import uuid
from pathlib import Path

from manifest import EXPECTED_ACCURACY
from run_metrics import (
    ACCURACY_FIELDS,
    MetricsError,
    SAMPLE_PROFILES,
    _accuracy_values,
    _read_csv,
    _require_source_fingerprint,
    _run,
    _runner_command,
    _validate_identity,
)
from source_fingerprint import source_identity


NATIVE_PRECISIONS = ("f32", "f64")


def _run_precision(
    executable: Path,
    output: Path,
    *,
    precision: str,
    run_id: str,
    revision: str,
    fingerprint: str,
    sample_mode: str,
) -> None:
    command = [
        *_runner_command(executable),
        "--precision",
        precision,
        "--output",
        str(output),
        "--run-id",
        run_id,
        "--source-revision",
        revision,
        "--sample-mode",
        sample_mode,
    ]
    configuration, return_code = _run(
        command,
        "accuracy",
        sample_mode=sample_mode,
        samples=SAMPLE_PROFILES[sample_mode]["accuracy_samples"],
        trials=0,
    )
    _require_source_fingerprint(configuration, fingerprint, f"native/{precision}")

    partial = output.with_name(output.stem + ".partial" + output.suffix)
    evidence = partial if partial.is_file() else output
    rows = _read_csv(
        evidence,
        ACCURACY_FIELDS,
        allow_partial=evidence == partial,
    )
    _validate_identity(
        rows,
        evidence,
        run_id=run_id,
        revision=revision,
        fingerprint=fingerprint,
        precision=precision,
    )
    observed = {
        (row["group"], row["operation"], row["domain"])
        for row in rows
    }
    expected = EXPECTED_ACCURACY[precision]
    if observed != expected:
        missing = sorted(expected - observed)
        extra = sorted(observed - expected)
        raise MetricsError(
            f"{evidence}: native accuracy manifest mismatch "
            f"(missing {missing[:5]}; unexpected {extra[:5]})"
        )
    for row in rows:
        _accuracy_values(row, evidence)
    if return_code:
        raise MetricsError(f"accuracy/{precision}: runner exited with status {return_code}")


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--accuracy", required=True, type=Path)
    parser.add_argument(
        "--output-root",
        type=Path,
        default=Path(__file__).resolve().parents[3] / "build" / "metrics" / "native",
        help="Parent for an invocation-specific result directory.",
    )
    parser.add_argument(
        "--sample-mode",
        choices=tuple(SAMPLE_PROFILES),
        default="full",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = Path(__file__).resolve().parents[3]
    run_id = uuid.uuid4().hex
    identity = source_identity(root)
    revision = identity.revision
    run_directory = args.output_root.resolve() / run_id
    run_directory.mkdir(parents=True, exist_ok=False)

    failures = []
    for precision in NATIVE_PRECISIONS:
        output = run_directory / f"{precision}_accuracy.csv"
        try:
            _run_precision(
                args.accuracy.resolve(),
                output,
                precision=precision,
                run_id=run_id,
                revision=revision,
                fingerprint=identity.fingerprint,
                sample_mode=args.sample_mode,
            )
        except (MetricsError, OSError) as error:
            print(f"{precision}: {error}", file=sys.stderr)
            failures.append(precision)

    if failures:
        print(
            f"native accuracy failed for {', '.join(failures)}; "
            f"complete streamed evidence remains in {run_directory}",
            file=sys.stderr,
        )
        return 1

    print(f"native accuracy passed; detailed CSVs are in {run_directory}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
