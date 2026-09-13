#!/usr/bin/env python3
"""Collect accuracy-only evidence: native baselines or fixed constexpr algorithms."""

from __future__ import annotations

import argparse
import sys
import uuid
from pathlib import Path
from typing import Mapping

from manifest import EXPECTED_ACCURACY, normalize_operations, select_operations
from run_metrics import (
    ACCURACY_FIELDS,
    MetricsError,
    SAMPLE_PROFILES,
    _accuracy_values,
    _benchmark_host_identity,
    _preflight_runners,
    _publication_lock,
    _publish_transaction,
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
FIXED_PRECISIONS = ("f32", "f64", "dd", "qd")
FIXED_POLICY = "fixed-constexpr-mpfr-v1"


def selected_precisions(fixed_constexpr: bool, operations: tuple[str, ...] = ()) -> tuple[str, ...]:
    return tuple(
        precision for precision in (FIXED_PRECISIONS if fixed_constexpr else NATIVE_PRECISIONS)
        if select_operations(EXPECTED_ACCURACY[precision], operations)
    )


def evidence_name(precision: str, fixed_constexpr: bool = False) -> str:
    suffix = "_fixed_constexpr" if fixed_constexpr else ""
    return f"{precision}{suffix}_accuracy.csv"


def metadata_name(fixed_constexpr: bool = False) -> str:
    return "fixed_constexpr_accuracy_run.json" if fixed_constexpr else "native_accuracy_run.json"


def threshold_policy(fixed_constexpr: bool, consumer_mode: str) -> str:
    return "gating" if fixed_constexpr and consumer_mode == "strict" else "advisory"


def require_execution_profile(
    configuration: Mapping[str, Mapping[str, str]],
    fixed_constexpr: bool,
    source: Path | str,
) -> Mapping[str, Mapping[str, str]]:
    expected = "on" if fixed_constexpr else "off"
    if configuration["consumer"]["simulated-consteval"] != expected:
        raise MetricsError(f"{source}: expected simulated-consteval={expected}")
    if fixed_constexpr and any(value != "off" for value in configuration["harness"].values()):
        raise MetricsError(f"{source}: fixed constexpr requires FLTX-only runners")
    return configuration


def _validate_accuracy_evidence(
    evidence: Path,
    *,
    precision: str,
    run_id: str,
    revision: str,
    fingerprint: str,
    operations: tuple[str, ...] = (),
    allow_partial: bool = False,
    fixed_constexpr: bool = False,
    consumer_mode: str = "strict",
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
    expected = select_operations(EXPECTED_ACCURACY[precision], operations)
    if observed != expected:
        missing = sorted(expected - observed)
        extra = sorted(observed - expected)
        raise MetricsError(
            f"{evidence}: {'fixed constexpr' if fixed_constexpr else 'native'} accuracy manifest mismatch "
            f"(missing {missing[:5]}; unexpected {extra[:5]})"
        )
    for row in rows:
        _accuracy_values(row, evidence)
        expected_implementation = "native" if precision in NATIVE_PRECISIONS else "fltx"
        if row["implementation"] != expected_implementation:
            raise MetricsError(f"{evidence}: unexpected accuracy implementation")
        if row["special_support"] not in {"Both", "Inf", "NaN", "No", "-"}:
            raise MetricsError(f"{evidence}: malformed special_support category")
        if row["signed_zero_support"] not in {"yes", "no", "-"}:
            raise MetricsError(f"{evidence}: malformed signed_zero_support category")
        if threshold_policy(fixed_constexpr, consumer_mode) == "gating" and row["pass"] != "yes":
            raise MetricsError(f"{evidence}: fixed constexpr accuracy threshold failed")


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
    operations: tuple[str, ...] = (),
    fixed_constexpr: bool = False,
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
        *(["--advisory"] if threshold_policy(fixed_constexpr, consumer_mode) == "advisory" else []),
    ]
    if operations:
        native_operations = sorted({
            operation
            for _, operation, _ in select_operations(
                EXPECTED_ACCURACY[precision], operations,
            )
        })
        if not native_operations:
            raise MetricsError(
                f"accuracy/{precision}: requested operations have no "
                "accuracy coverage"
            )
        for operation in native_operations:
            command.extend(("--operation", operation))
    configuration, return_code = _run(
        command,
        "accuracy",
        sample_mode=sample_mode,
        samples=SAMPLE_PROFILES[sample_mode]["accuracy_samples"],
        trials=0,
    )
    _require_source_fingerprint(configuration, fingerprint, f"accuracy/{precision}")
    configuration = require_consumer_mode(
        configuration,
        consumer_mode,
        f"accuracy/{precision}",
    )
    require_execution_profile(configuration, fixed_constexpr, f"accuracy/{precision}")

    partial = output.with_name(output.stem + ".partial" + output.suffix)
    evidence = partial if partial.is_file() else output
    _validate_accuracy_evidence(
        evidence,
        precision=precision,
        run_id=run_id,
        revision=revision,
        fingerprint=fingerprint,
        operations=operations,
        allow_partial=evidence == partial,
        fixed_constexpr=fixed_constexpr,
        consumer_mode=consumer_mode,
    )
    if return_code:
        raise MetricsError(f"accuracy/{precision}: runner exited with status {return_code}")
    if evidence == partial:
        raise MetricsError(f"accuracy/{precision}: runner did not commit its final CSV")
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
    operations: tuple[str, ...] = (),
    fixed_constexpr: bool = False,
) -> dict[str, object]:
    result: dict[str, object] = {
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
    if operations:
        result["operations"] = list(operations)
    if fixed_constexpr:
        result["execution_profile"] = "fixed_constexpr"
    return result


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
    operations: tuple[str, ...] = (),
    fixed_constexpr: bool = False,
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
        "precisions": list(selected_precisions(fixed_constexpr, operations)),
        "policy": FIXED_POLICY if fixed_constexpr else NATIVE_POLICY,
        "thresholds": threshold_policy(fixed_constexpr, consumer_mode),
    }
    if fixed_constexpr:
        required["execution_profile"] = "fixed_constexpr"
    for field, expected in required.items():
        if metadata.get(field) != expected:
            raise MetricsError(
                f"{metadata_path}: reusable {field} mismatch "
                f"({metadata.get(field)!r} != {expected!r})"
            )
    recorded_operations = metadata.get("operations", [])
    if recorded_operations != list(operations):
        raise MetricsError(
            f"{metadata_path}: reusable operations mismatch "
            f"({recorded_operations!r} != {list(operations)!r})"
        )
    if metadata.get("configuration") != configuration:
        raise MetricsError(
            f"{metadata_path}: reusable build configuration changed"
        )
    require_execution_profile(configuration, fixed_constexpr, metadata_path)
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
        evidence_name(precision, fixed_constexpr)
        for precision in selected_precisions(fixed_constexpr, operations)
    }
    _verify_output_hashes(
        run_directory,
        metadata.get("outputs"),
        expected_outputs,
        metadata_path,
    )
    for precision in selected_precisions(fixed_constexpr, operations):
        _validate_accuracy_evidence(
            run_directory / evidence_name(precision, fixed_constexpr),
            precision=precision,
            run_id=run_id,
            revision=revision,
            fingerprint=fingerprint,
            operations=operations,
            fixed_constexpr=fixed_constexpr,
            consumer_mode=consumer_mode,
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
        operations=operations,
        fixed_constexpr=fixed_constexpr,
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
    operations: tuple[str, ...] = (),
    fixed_constexpr: bool = False,
) -> tuple[dict[str, object] | None, str]:
    candidates = list(output_root.glob(f"*/{metadata_name(fixed_constexpr)}"))
    candidates.sort(key=lambda path: path.stat().st_mtime, reverse=True)
    if not candidates:
        return None, "no previous complete accuracy evidence exists"

    rejection = "no compatible accuracy evidence exists"
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
                    operations=operations,
                    fixed_constexpr=fixed_constexpr,
                ),
                "",
            )
        except (MetricsError, OSError) as error:
            rejection = str(error)
    return None, rejection


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--accuracy", required=True, type=Path)
    parser.add_argument("--fixed_constexpr", "--fixed-constexpr", action="store_true")
    parser.add_argument("--canonical-publication", action="store_true", help=argparse.SUPPRESS)
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
        "--operation",
        dest="operations",
        action="append",
        metavar="NAME",
        help="Select an exact operation name; repeat to run their union.",
    )
    parser.add_argument(
        "--reuse-compatible",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    parser.add_argument("--result-file", type=Path)
    args = parser.parse_args(argv)
    try:
        args.operations = normalize_operations(args.operations)
    except ValueError as error:
        parser.error(str(error))
    if args.operations and not any(
        select_operations(EXPECTED_ACCURACY[precision], args.operations)
        for precision in selected_precisions(args.fixed_constexpr)
    ):
        parser.error("requested operations have no accuracy coverage in this profile")
    return args


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = Path(__file__).resolve().parents[3]
    identity = source_identity(root)
    revision = identity.revision
    accuracy = args.accuracy.resolve()
    output_root = args.output_root.resolve()
    fixed = args.fixed_constexpr
    precisions = selected_precisions(fixed, args.operations)
    label = "fixed constexpr" if fixed else "native"
    canonical = (root / "validation" / "metrics" / "data").resolve()
    if fixed and output_root.is_relative_to(canonical) and not args.canonical_publication:
        print("fixed constexpr canonical output requires --publish", file=sys.stderr)
        return 1
    if args.canonical_publication and (not fixed or args.sample_mode != "full" or args.operations):
        print("fixed constexpr publication requires a complete full profile", file=sys.stderr)
        return 1
    if args.operations and not output_root.is_relative_to(root / "build"):
        print("native accuracy failed: operation-filtered output must stay under build/", file=sys.stderr)
        return 1
    try:
        provenance = _run_provenance((("accuracy", accuracy),))
        configuration = _preflight_runners(
            (("accuracy", accuracy),),
            fingerprint=identity.fingerprint,
            platform=None,
            architecture=None,
            compiler=None,
            publishable=fixed and (args.sample_mode == "full" or args.canonical_publication),
        )
        configuration = require_consumer_mode(
            configuration,
            args.consumer_mode,
            "accuracy preflight",
        )
        require_execution_profile(configuration, fixed, "accuracy preflight")
        if args.canonical_publication and revision == "unknown":
            raise MetricsError("publication requires a known source revision")
        platform, architecture, compiler = infer_canonical_target(
            configuration,
            "accuracy preflight",
        )
    except (MetricsError, OSError) as error:
        print(f"{label} accuracy failed: {error}", file=sys.stderr)
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
            operations=args.operations,
            fixed_constexpr=fixed,
        )
        if reusable is not None:
            if args.result_file is not None:
                _write_json(args.result_file.resolve(), reusable)
            print(
                f"reusing compatible {label} {args.sample_mode} accuracy run "
                f"{reusable['run_id']} for {reusable['target']}; "
                f"{'/'.join(precisions)} phases skipped",
                flush=True,
            )
            return 0
        print(
            f"no reusable {label} {args.sample_mode} accuracy evidence: "
            f"{rejection}; running {'/'.join(precisions)} accuracy",
            flush=True,
        )

    run_id = uuid.uuid4().hex
    final_directory = output_root / run_id
    run_directory = output_root / ".staging" / run_id if fixed else final_directory
    run_directory.mkdir(parents=True, exist_ok=False)

    failures = []
    for precision in precisions:
        output = run_directory / evidence_name(precision, fixed)
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
                operations=args.operations,
                fixed_constexpr=fixed,
            )
            configuration = _require_matching_configuration(
                configuration,
                observed_configuration,
                f"accuracy/{precision}",
            )
        except (MetricsError, OSError) as error:
            print(f"{precision}: {error}", file=sys.stderr)
            failures.append(precision)

    if failures:
        print(
            f"{label} accuracy failed for {', '.join(failures)}; "
            f"complete streamed evidence remains in {run_directory}",
            file=sys.stderr,
        )
        return 1

    outputs = {
        path.name: _sha256_file(path)
        for path in sorted(run_directory.glob("*_accuracy.csv"))
    }
    metadata = run_directory / metadata_name(fixed)
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
            "precisions": list(precisions),
            "operations": list(args.operations),
            "policy": FIXED_POLICY if fixed else NATIVE_POLICY,
            "thresholds": threshold_policy(fixed, args.consumer_mode),
            **({"execution_profile": "fixed_constexpr"} if fixed else {}),
            "configuration": configuration,
            "host": provenance["host"],
            "executables": provenance["executables"],
            "outputs": outputs,
        },
    )
    if fixed:
        try:
            with _publication_lock(output_root / f".{platform}_{architecture}_{compiler}_fixed_constexpr.publish.lock"):
                _publish_transaction(
                    [(run_directory / name, final_directory / name) for name in outputs],
                    metadata,
                    final_directory / metadata.name,
                )
        except (MetricsError, OSError) as error:
            print(f"fixed constexpr publication failed: {error}", file=sys.stderr)
            return 1
        run_directory = final_directory
        metadata = run_directory / metadata.name
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
                operations=args.operations,
                fixed_constexpr=fixed,
            ),
        )
    print(
        f"{label} accuracy recorded; detailed CSVs are in {run_directory}",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
