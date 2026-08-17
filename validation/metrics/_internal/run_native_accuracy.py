#!/usr/bin/env python3
"""Record native f32/f64 accuracy baselines without publishing summary tables."""

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
    _benchmark_host_identity,
    _preflight_runners,
    _read_metadata,
    _require_matching_configuration,
    _read_csv,
    _require_source_fingerprint,
    _run,
    _run_provenance,
    _runner_command,
    _sha256_file,
    _validate_identity,
    _validate_provenance,
    _verify_output_hashes,
    _write_json,
    infer_canonical_target,
    require_consumer_mode,
)
from source_fingerprint import source_identity


NATIVE_PRECISIONS = ("f32", "f64")
NATIVE_SCHEMA_VERSION = 2
NATIVE_POLICY = "native-float-libm-baseline-v1"


def _validate_accuracy_evidence(
    evidence: Path,
    *,
    precision: str,
    run_id: str,
    revision: str,
    fingerprint: str,
    allow_partial: bool = False,
) -> None:
    rows = _read_csv(
        evidence,
        ACCURACY_FIELDS,
        allow_partial=allow_partial,
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


def _run_precision(
    executable: Path,
    output: Path,
    *,
    precision: str,
    run_id: str,
    revision: str,
    fingerprint: str,
    sample_mode: str,
    consumer_mode: str,
) -> dict[str, dict[str, str]]:
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
        "--advisory",
    ]
    configuration, return_code = _run(
        command,
        "accuracy",
        sample_mode=sample_mode,
        samples=SAMPLE_PROFILES[sample_mode]["accuracy_samples"],
        trials=0,
    )
    _require_source_fingerprint(configuration, fingerprint, f"native/{precision}")
    configuration = require_consumer_mode(
        configuration,
        consumer_mode,
        f"native/{precision}",
    )

    partial = output.with_name(output.stem + ".partial" + output.suffix)
    evidence = partial if partial.is_file() else output
    _validate_accuracy_evidence(
        evidence,
        precision=precision,
        run_id=run_id,
        revision=revision,
        fingerprint=fingerprint,
        allow_partial=evidence == partial,
    )
    if return_code:
        raise MetricsError(f"accuracy/{precision}: runner exited with status {return_code}")
    return configuration


def _result_handoff(
    *,
    run_id: str,
    sample_mode: str,
    consumer_mode: str,
    platform: str,
    architecture: str,
    compiler: str,
    run_directory: Path,
    metadata: Path,
) -> dict[str, object]:
    return {
        "schema_version": NATIVE_SCHEMA_VERSION,
        "run_id": run_id,
        "sample_mode": sample_mode,
        "consumer_mode": consumer_mode,
        "platform": platform,
        "architecture": architecture,
        "compiler": compiler,
        "target": f"{platform}/{architecture}/{compiler}",
        "run_directory": str(run_directory.resolve()),
        "metadata": str(metadata.resolve()),
    }


def _validate_reusable_candidate(
    metadata_path: Path,
    *,
    sample_mode: str,
    consumer_mode: str,
    platform: str,
    architecture: str,
    compiler: str,
    fingerprint: str,
    configuration: dict[str, dict[str, str]],
    host: object,
) -> dict[str, object]:
    metadata = _read_metadata(metadata_path)
    required = {
        "schema_version": NATIVE_SCHEMA_VERSION,
        "status": "complete",
        "source_fingerprint": fingerprint,
        "sample_mode": sample_mode,
        "accuracy_samples": SAMPLE_PROFILES[sample_mode]["accuracy_samples"],
        "consumer_mode": consumer_mode,
        "platform": platform,
        "architecture": architecture,
        "compiler": compiler,
        "target": f"{platform}/{architecture}/{compiler}",
        "precisions": list(NATIVE_PRECISIONS),
        "policy": NATIVE_POLICY,
        "thresholds": "advisory",
    }
    for field, expected in required.items():
        if metadata.get(field) != expected:
            raise MetricsError(
                f"{metadata_path}: reusable {field} mismatch "
                f"({metadata.get(field)!r} != {expected!r})"
            )
    if metadata.get("configuration") != configuration:
        raise MetricsError(
            f"{metadata_path}: reusable build configuration changed"
        )
    _validate_provenance(
        metadata.get("host"),
        metadata.get("executables"),
        metadata_path,
    )
    executables = metadata.get("executables")
    if not isinstance(executables, dict) or set(executables) != {"accuracy"}:
        raise MetricsError(
            f"{metadata_path}: reusable executable provenance is incomplete"
        )
    if _benchmark_host_identity(
        metadata.get("host")
    ) != _benchmark_host_identity(host):
        raise MetricsError(f"{metadata_path}: reusable host identity changed")

    run_id = metadata.get("run_id")
    revision = metadata.get("source_revision")
    if not isinstance(run_id, str) or not run_id:
        raise MetricsError(f"{metadata_path}: reusable run ID is missing")
    if not isinstance(revision, str) or not revision:
        raise MetricsError(f"{metadata_path}: reusable source revision is missing")

    run_directory = metadata_path.parent
    expected_outputs = {
        f"{precision}_accuracy.csv" for precision in NATIVE_PRECISIONS
    }
    _verify_output_hashes(
        run_directory,
        metadata.get("outputs"),
        expected_outputs,
        metadata_path,
    )
    for precision in NATIVE_PRECISIONS:
        _validate_accuracy_evidence(
            run_directory / f"{precision}_accuracy.csv",
            precision=precision,
            run_id=run_id,
            revision=revision,
            fingerprint=fingerprint,
        )

    return _result_handoff(
        run_id=run_id,
        sample_mode=sample_mode,
        consumer_mode=consumer_mode,
        platform=platform,
        architecture=architecture,
        compiler=compiler,
        run_directory=run_directory,
        metadata=metadata_path,
    )


def _find_reusable_run(
    output_root: Path,
    *,
    sample_mode: str,
    consumer_mode: str,
    platform: str,
    architecture: str,
    compiler: str,
    fingerprint: str,
    configuration: dict[str, dict[str, str]],
    host: object,
) -> tuple[dict[str, object] | None, str]:
    candidates = list(output_root.glob("*/native_accuracy_run.json"))
    candidates.sort(key=lambda path: path.stat().st_mtime, reverse=True)
    if not candidates:
        return None, "no previous complete native evidence exists"

    rejection = "no compatible native evidence exists"
    for candidate in candidates:
        try:
            return (
                _validate_reusable_candidate(
                    candidate,
                    sample_mode=sample_mode,
                    consumer_mode=consumer_mode,
                    platform=platform,
                    architecture=architecture,
                    compiler=compiler,
                    fingerprint=fingerprint,
                    configuration=configuration,
                    host=host,
                ),
                "",
            )
        except (MetricsError, OSError) as error:
            rejection = str(error)
    return None, rejection


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
    parser.add_argument(
        "--consumer-mode",
        choices=("strict", "fastmath"),
        default="strict",
    )
    parser.add_argument(
        "--reuse-compatible",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    parser.add_argument("--result-file", type=Path)
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = Path(__file__).resolve().parents[3]
    identity = source_identity(root)
    revision = identity.revision
    accuracy = args.accuracy.resolve()
    output_root = args.output_root.resolve()
    try:
        provenance = _run_provenance((("accuracy", accuracy),))
        configuration = _preflight_runners(
            (("accuracy", accuracy),),
            fingerprint=identity.fingerprint,
            platform=None,
            architecture=None,
            compiler=None,
            publishable=False,
        )
        configuration = require_consumer_mode(
            configuration,
            args.consumer_mode,
            "native accuracy preflight",
        )
        platform, architecture, compiler = infer_canonical_target(
            configuration,
            "native accuracy preflight",
        )
    except (MetricsError, OSError) as error:
        print(f"native accuracy failed: {error}", file=sys.stderr)
        return 1

    if args.reuse_compatible:
        reusable, rejection = _find_reusable_run(
            output_root,
            sample_mode=args.sample_mode,
            consumer_mode=args.consumer_mode,
            platform=platform,
            architecture=architecture,
            compiler=compiler,
            fingerprint=identity.fingerprint,
            configuration=configuration,
            host=provenance["host"],
        )
        if reusable is not None:
            if args.result_file is not None:
                _write_json(args.result_file.resolve(), reusable)
            print(
                f"reusing compatible native {args.sample_mode} accuracy run "
                f"{reusable['run_id']} for {reusable['target']}; "
                "f32/f64 phases skipped",
                flush=True,
            )
            return 0
        print(
            f"no reusable native {args.sample_mode} accuracy evidence: "
            f"{rejection}; running f32/f64 accuracy",
            flush=True,
        )

    run_id = uuid.uuid4().hex
    run_directory = output_root / run_id
    run_directory.mkdir(parents=True, exist_ok=False)

    failures = []
    for precision in NATIVE_PRECISIONS:
        output = run_directory / f"{precision}_accuracy.csv"
        try:
            observed_configuration = _run_precision(
                accuracy,
                output,
                precision=precision,
                run_id=run_id,
                revision=revision,
                fingerprint=identity.fingerprint,
                sample_mode=args.sample_mode,
                consumer_mode=args.consumer_mode,
            )
            configuration = _require_matching_configuration(
                configuration,
                observed_configuration,
                f"native/{precision}",
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

    outputs = {
        path.name: _sha256_file(path)
        for path in sorted(run_directory.glob("*_accuracy.csv"))
    }
    metadata = run_directory / "native_accuracy_run.json"
    _write_json(
        metadata,
        {
            "schema_version": NATIVE_SCHEMA_VERSION,
            "status": "complete",
            "run_id": run_id,
            "source_revision": revision,
            "source_fingerprint": identity.fingerprint,
            "sample_mode": args.sample_mode,
            "accuracy_samples": SAMPLE_PROFILES[
                args.sample_mode
            ]["accuracy_samples"],
            "consumer_mode": args.consumer_mode,
            "platform": platform,
            "architecture": architecture,
            "compiler": compiler,
            "target": f"{platform}/{architecture}/{compiler}",
            "precisions": list(NATIVE_PRECISIONS),
            "policy": NATIVE_POLICY,
            "thresholds": "advisory",
            "configuration": configuration,
            "host": provenance["host"],
            "executables": provenance["executables"],
            "outputs": outputs,
        },
    )
    if args.result_file is not None:
        _write_json(
            args.result_file.resolve(),
            _result_handoff(
                run_id=run_id,
                sample_mode=args.sample_mode,
                consumer_mode=args.consumer_mode,
                platform=platform,
                architecture=architecture,
                compiler=compiler,
                run_directory=run_directory,
                metadata=metadata,
            ),
        )
    print(
        f"native accuracy baseline recorded; detailed CSVs are in {run_directory}",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
