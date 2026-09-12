#!/usr/bin/env python3
"""Incrementally build runners, reuse compatible evidence, and render reports."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import Mapping, Sequence

VALIDATION_INTERNAL = Path(__file__).resolve().parents[2] / "_internal"
sys.path.insert(0, str(VALIDATION_INTERNAL))

from preset_support import (
    PipelineError,
    PresetSelection,
    msvc_environment,
    msvc_target_architecture,
    needs_msvc_environment,
    read_json,
    resolve_preset,
)

import build_profile_comparison
from build_tables import REPORT_EXCLUDED_OPERATIONS, Target
from manifest import EXPECTED_ACCURACY, normalize_operations, operation_key, select_operations
import run_metrics


RUNNER_SETS = {
    "strict": ("fltx_accuracy", "fltx_benchmark"),
    "fastmath": ("fltx_accuracy_fastmath", "fltx_benchmark_fastmath"),
}
RUNNER_TARGETS = tuple(
    target
    for targets in RUNNER_SETS.values()
    for target in targets
)
DEFAULT_WORKFLOW = "standard"


@dataclass(frozen=True)
class MetricsWorkflow:
    sample_mode: str


@dataclass(frozen=True)
class MetricsPaths:
    data: Path
    generated: Path


METRICS_WORKFLOWS = {
    "quick": MetricsWorkflow("small"),
    "standard": MetricsWorkflow("standard"),
    "full": MetricsWorkflow("full"),
}


def prepare_file_api_query(binary_dir: Path) -> None:
    query = binary_dir / ".cmake" / "api" / "v1" / "query" / "client-fltx-metrics"
    query.mkdir(parents=True, exist_ok=True)
    (query / "query.json").write_text(
        json.dumps(
            {"requests": [{"kind": "codemodel", "version": {"major": 2}}]},
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )


def _codemodel_path(binary_dir: Path) -> Path:
    reply = binary_dir / ".cmake" / "api" / "v1" / "reply"
    indexes = sorted(
        reply.glob("index-*.json"),
        key=lambda path: path.stat().st_mtime_ns,
        reverse=True,
    )
    for index_path in indexes:
        index = read_json(index_path)
        client = index.get("reply", {})
        if not isinstance(client, dict):
            continue
        response = client.get("client-fltx-metrics", {})
        if not isinstance(response, dict):
            continue
        query = response.get("query.json", {})
        if not isinstance(query, dict):
            continue
        responses = query.get("responses", [])
        if not isinstance(responses, list):
            continue
        for item in responses:
            if (
                isinstance(item, dict)
                and item.get("kind") == "codemodel"
                and isinstance(item.get("jsonFile"), str)
            ):
                return reply / item["jsonFile"]
    raise PipelineError(
        f"CMake did not produce a File API codemodel under {reply}"
    )


def discover_runner_artifacts(
    binary_dir: Path,
    configuration: str | None,
) -> dict[str, Path]:
    codemodel_path = _codemodel_path(binary_dir)
    codemodel = read_json(codemodel_path)
    configurations = codemodel.get("configurations")
    if not isinstance(configurations, list) or not configurations:
        raise PipelineError(f"{codemodel_path}: missing configurations")

    selected: dict[str, object] | None = None
    if configuration is not None:
        selected = next(
            (
                value
                for value in configurations
                if isinstance(value, dict)
                and str(value.get("name", "")).casefold() == configuration.casefold()
            ),
            None,
        )
        if selected is None:
            names = [
                str(value.get("name", ""))
                for value in configurations
                if isinstance(value, dict)
            ]
            raise PipelineError(
                f"CMake File API has no {configuration!r} configuration "
                f"(available: {', '.join(names)})"
            )
    elif len(configurations) == 1 and isinstance(configurations[0], dict):
        selected = configurations[0]
    else:
        raise PipelineError(
            "multi-config build preset must specify its configuration"
        )
    assert selected is not None

    targets = selected.get("targets")
    if not isinstance(targets, list):
        raise PipelineError(f"{codemodel_path}: selected configuration has no targets")
    reply = codemodel_path.parent
    artifacts: dict[str, Path] = {}
    for target_name in RUNNER_TARGETS:
        references = [
            value
            for value in targets
            if isinstance(value, dict) and value.get("name") == target_name
        ]
        if len(references) != 1 or not isinstance(references[0].get("jsonFile"), str):
            raise PipelineError(
                f"CMake File API expected one {target_name!r} target, "
                f"found {len(references)}"
            )
        target_path = reply / references[0]["jsonFile"]
        target = read_json(target_path)
        if target.get("type") != "EXECUTABLE":
            raise PipelineError(f"{target_path}: {target_name} is not an executable")
        values = target.get("artifacts")
        if not isinstance(values, list) or not values:
            raise PipelineError(f"{target_path}: {target_name} has no artifact")
        candidates = [
            value["path"]
            for value in values
            if isinstance(value, dict) and isinstance(value.get("path"), str)
        ]
        name_on_disk = target.get("nameOnDisk")
        if isinstance(name_on_disk, str):
            named = [value for value in candidates if Path(value).name == name_on_disk]
            if named:
                candidates = named
        if len(candidates) != 1:
            raise PipelineError(
                f"{target_path}: cannot identify the primary {target_name} artifact"
            )
        artifact = Path(candidates[0])
        if not artifact.is_absolute():
            artifact = binary_dir / artifact
        artifact = artifact.resolve()
        if not artifact.is_file():
            raise PipelineError(f"built runner artifact does not exist: {artifact}")
        artifacts[target_name] = artifact
    return artifacts


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


def metrics_paths(
    root: Path,
    workflow: str,
    output_root: Path | None = None,
    *,
    publish: bool = False,
    operations: tuple[str, ...] = (),
) -> MetricsPaths:
    policy = METRICS_WORKFLOWS.get(workflow)
    if policy is None:
        raise PipelineError(f"unsupported metrics workflow {workflow!r}")

    metrics = (root / "validation" / "metrics").resolve()
    canonical = MetricsPaths(
        data=metrics / "data",
        generated=metrics / "generated",
    )
    if publish:
        if operations:
            raise PipelineError("--publish cannot be combined with --operation")
        if output_root is not None:
            raise PipelineError("--publish cannot be combined with --output-root")
        return canonical

    selected = (
        output_root.resolve()
        if output_root is not None
        else root.resolve() / "build" / "metrics" / workflow
        if operations
        else metrics / "_unversioned" / workflow
    )
    if operations:
        if not selected.is_relative_to(root.resolve() / "build"):
            raise PipelineError("operation-filtered output must stay under build/")
        selected = selected / "operations" / operation_key(operations)
    paths = MetricsPaths(
        data=selected / "data",
        generated=selected / "generated",
    )
    if paths == canonical:
        raise PipelineError("canonical metrics output requires --publish")
    return paths


def existing_runner_artifacts(
    root: Path,
    selection: PresetSelection,
    sample_mode: str,
    environment: Mapping[str, str] | None = None,
    *,
    publish: bool = False,
) -> dict[str, Path] | None:
    """Return source-current runner artifacts without invoking CMake."""

    try:
        artifacts = discover_runner_artifacts(
            selection.binary_dir,
            selection.configuration,
        )
        identities = set()
        for consumer_mode, (accuracy_target, benchmark_target) in RUNNER_SETS.items():
            identities.add(
                run_metrics.validate_runner_artifacts(
                    root,
                    {
                        "fltx_accuracy": artifacts[accuracy_target],
                        "fltx_benchmark": artifacts[benchmark_target],
                    },
                    sample_mode=sample_mode,
                    consumer_mode=consumer_mode,
                    environment=environment,
                    canonical_publication=publish,
                )
            )
        if len(identities) != 1:
            raise PipelineError(
                "strict and consumer-fast-math runners identify different targets"
            )
        platform_name, architecture_name, compiler_name = next(iter(identities))
    except (PipelineError, run_metrics.MetricsError, OSError) as error:
        print(
            f"existing metrics runners cannot be reused: {error}; "
            "configuring/building",
            flush=True,
        )
        return None

    print(
        "existing metrics runners match the current source fingerprint "
        f"for {platform_name}/{architecture_name}/{compiler_name}; "
        "CMake configure/build skipped",
        flush=True,
    )
    return artifacts


def run_pipeline(
    root: Path,
    preset: str,
    *,
    workflow: str = DEFAULT_WORKFLOW,
    output_root: Path | None = None,
    publish: bool = False,
    force_rerun: bool = False,
    consumer_mode: str = "strict",
    operations: tuple[str, ...] = (),
) -> list[Path]:
    root = root.resolve()
    try:
        operations = normalize_operations(operations)
    except ValueError as error:
        raise PipelineError(str(error)) from error
    excluded = {operation for _, operation in REPORT_EXCLUDED_OPERATIONS} & set(operations)
    if excluded:
        raise PipelineError(
            f"{', '.join(sorted(excluded))} is a harness baseline excluded from reports; "
            "use the internal run_metrics.py command for baseline-only evidence"
        )
    operation_arguments = [
        value for operation in operations for value in ("--operation", operation)
    ]
    if consumer_mode == "all":
        requested_modes = tuple(RUNNER_SETS)
    elif consumer_mode in RUNNER_SETS:
        requested_modes = (consumer_mode,)
    else:
        raise PipelineError(f"unsupported consumer mode {consumer_mode!r}")
    policy = METRICS_WORKFLOWS.get(workflow)
    if policy is None:
        raise PipelineError(f"unsupported metrics workflow {workflow!r}")
    selected_sample_mode = policy.sample_mode
    selected_paths = metrics_paths(
        root,
        workflow,
        output_root,
        publish=publish,
        operations=operations,
    )
    selection = resolve_preset(root, preset)
    environment = selection.environment
    artifacts = existing_runner_artifacts(
        root,
        selection,
        selected_sample_mode,
        environment,
        publish=publish,
    )
    if artifacts is None:
        cmake_environment: Mapping[str, str] | None = None
        if needs_msvc_environment(selection):
            cmake_environment = msvc_environment(
                msvc_target_architecture(selection),
                parent_environment=selection.environment,
            )
            environment = cmake_environment
        prepare_file_api_query(selection.binary_dir)
        try:
            _run(
                [
                    "cmake",
                    "--preset",
                    selection.configure_name,
                    "-DFLTX_METRICS_EXTERNAL_COMPARISONS=ON",
                ],
                root,
                cmake_environment,
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
                *RUNNER_TARGETS,
            ],
            root,
            cmake_environment,
        )
        artifacts = discover_runner_artifacts(
            selection.binary_dir,
            selection.configuration,
        )

    generated: list[Path] = []
    output = selected_paths.generated
    comparison_target: str | None = None
    comparison_input_root: str | None = None
    comparison_identity: tuple[str, str, str] | None = None
    for mode in requested_modes:
        accuracy_target, benchmark_target = RUNNER_SETS[mode]
        handoff = (
            root
            / "build"
            / "metrics"
            / "pipeline"
            / uuid.uuid4().hex
            / f"{mode}_run_result.json"
        )
        _run(
            [
                sys.executable,
                str(
                    root
                    / "validation"
                    / "metrics"
                    / "_internal"
                    / "run_metrics.py"
                ),
                "--accuracy",
                str(artifacts[accuracy_target]),
                "--benchmark",
                str(artifacts[benchmark_target]),
                "--consumer-mode",
                mode,
                "--sample-mode",
                selected_sample_mode,
                "--output-root",
                str(selected_paths.data),
                *(
                    ["--canonical-publication"]
                    if publish
                    else []
                ),
                "--result-file",
                str(handoff),
                *([] if force_rerun else ["--reuse-compatible"]),
                *operation_arguments,
            ],
            root,
            environment,
        )
        result = read_json(handoff)
        expected_fields = {
            "schema_version",
            "run_id",
            "sample_mode",
            "consumer_mode",
            "platform",
            "architecture",
            "compiler",
            "target",
            "input_root",
            "metadata",
        }
        if operations:
            expected_fields.add("operations")
        if (
            set(result) != expected_fields
            or result.get("sample_mode") != selected_sample_mode
            or result.get("consumer_mode") != mode
            or result.get("operations", []) != list(operations)
        ):
            raise PipelineError(f"{handoff}: invalid metrics result handoff")
        target = result.get("target")
        input_root = result.get("input_root")
        if not isinstance(target, str) or not isinstance(input_root, str):
            raise PipelineError(f"{handoff}: invalid metrics target or input root")
        identity = (
            str(result["platform"]),
            str(result["architecture"]),
            str(result["compiler"]),
        )
        if comparison_target is None:
            comparison_target = target
            comparison_input_root = input_root
            comparison_identity = identity
        elif (
            comparison_target != target
            or comparison_input_root != input_root
            or comparison_identity != identity
        ):
            raise PipelineError(
                "consumer modes produced incompatible metrics targets"
            )

        if select_operations(EXPECTED_ACCURACY["f32"], operations):
            run_native_baseline(
                root, artifacts[accuracy_target], selected_paths.data, handoff,
                result, selected_sample_mode, mode, force_rerun,
                operations, environment,
            )

        _run(
            [
                sys.executable,
                str(root / "validation" / "metrics" / "rebuild_tables.py"),
                "--input",
                input_root,
                "--output",
                str(output),
                "--targets",
                target,
                "--consumer-mode",
                mode,
            ],
            root,
            environment,
        )
        mode_suffix = run_metrics.consumer_mode_suffix(mode)
        mode_outputs = [
            output / "accuracy" / f"accuracy_table{mode_suffix}.svg",
            output / "performance" / f"performance_table{mode_suffix}.svg",
            output
            / "performance"
            / f"performance_table{mode_suffix}_compact.svg",
        ]
        for precision in run_metrics.PRECISIONS:
            base = (
                f"{result['platform']}_{result['architecture']}_"
                f"{result['compiler']}_{precision}"
                f"{mode_suffix}_overview"
            )
            mode_outputs.extend(
                (
                    output / "overview" / f"{base}.svg",
                    output / "overview" / f"{base}_compact.svg",
                )
            )
        missing = [path for path in mode_outputs if not path.is_file()]
        if missing:
            raise PipelineError(
                "renderers did not create expected outputs: "
                + ", ".join(str(path) for path in missing)
            )
        generated.extend(mode_outputs)

    assert comparison_target is not None
    assert comparison_input_root is not None
    assert comparison_identity is not None
    comparison_output = output / "profile_comparison"
    _run(
        [
            sys.executable,
            str(
                root
                / "validation"
                / "metrics"
                / "_internal"
                / "build_profile_comparison.py"
            ),
            "--input",
            comparison_input_root,
            "--output",
            str(comparison_output),
            "--target",
            comparison_target,
            *(["--required"] if consumer_mode == "all" else []),
        ],
        root,
        environment,
    )
    platform_name, architecture_name, compiler_name = comparison_identity
    comparison_outputs = build_profile_comparison.output_paths(
        comparison_output,
        Target(platform_name, architecture_name, compiler_name),
    )
    existing_comparisons = tuple(
        path for path in comparison_outputs if path.is_file()
    )
    if existing_comparisons and len(existing_comparisons) != len(
        comparison_outputs
    ):
        raise PipelineError(
            "profile comparison renderer created an incomplete output set"
        )
    if consumer_mode == "all" and not existing_comparisons:
        raise PipelineError(
            "profile comparison renderer did not create required outputs"
        )
    generated.extend(existing_comparisons)
    return generated


def run_native_baseline(
    root: Path,
    accuracy: Path,
    data: Path,
    handoff: Path,
    result: Mapping[str, object],
    sample_mode: str,
    mode: str,
    force_rerun: bool,
    operations: tuple[str, ...],
    environment: Mapping[str, str],
) -> None:
    native_handoff = handoff.with_name(f"{mode}_native_result.json")
    _run(
        [
            sys.executable,
            str(
                root
                / "validation"
                / "metrics"
                / "_internal"
                / "run_native_accuracy.py"
            ),
            "--accuracy",
            str(accuracy),
            "--consumer-mode",
            mode,
            "--sample-mode",
            sample_mode,
            "--output-root",
            str(data / "native" / mode),
            "--result-file",
            str(native_handoff),
            *([] if force_rerun else ["--reuse-compatible"]),
            *(value for operation in operations for value in ("--operation", operation)),
        ],
        root,
        environment,
    )
    native_result = read_json(native_handoff)
    native_fields = {
        "schema_version",
        "run_id",
        "sample_mode",
        "consumer_mode",
        "platform",
        "architecture",
        "compiler",
        "target",
        "run_directory",
        "metadata",
    }
    if operations:
        native_fields.add("operations")
    if (
        set(native_result) != native_fields
        or native_result.get("sample_mode") != sample_mode
        or native_result.get("consumer_mode") != mode
        or native_result.get("operations", []) != list(operations)
        or native_result.get("target") != result["target"]
        or native_result.get("platform") != result["platform"]
        or native_result.get("architecture") != result["architecture"]
        or native_result.get("compiler") != result["compiler"]
    ):
        raise PipelineError(f"{native_handoff}: invalid native result handoff")
    native_directory = Path(str(native_result["run_directory"]))
    native_outputs = [
        Path(str(native_result["metadata"])),
        *(native_directory / f"{precision}_accuracy.csv"
          for precision in ("f32", "f64")),
    ]
    missing_native = [path for path in native_outputs if not path.is_file()]
    if missing_native:
        raise PipelineError(
            "native baseline did not create complete evidence: "
            + ", ".join(str(path) for path in missing_native)
        )


def add_workflow_arguments(parser: argparse.ArgumentParser) -> None:
    """Add the metrics workflow arguments shared by public entry points."""

    modes = parser.add_mutually_exclusive_group()
    modes.add_argument(
        "--quick",
        action="store_const",
        const="quick",
        dest="workflow",
        help="run the half-sized small profile",
    )
    modes.add_argument(
        "--standard",
        action="store_const",
        const="standard",
        dest="workflow",
        help="run the standard development profile (default)",
    )
    modes.add_argument(
        "--full",
        action="store_const",
        const="full",
        dest="workflow",
        help="run the full profile",
    )
    parser.set_defaults(workflow=DEFAULT_WORKFLOW)
    parser.add_argument(
        "--operation", action="append", dest="operations", metavar="NAME",
        help="run an exact operation name; repeat to select several (development only)",
    )
    parser.add_argument(
        "--publish",
        action="store_true",
        help=(
            "publish the selected profile to validation/metrics/data and "
            "validation/metrics/generated; cannot be combined with --output-root"
        ),
    )
    parser.add_argument(
        "--output-root",
        type=Path,
        metavar="PATH",
        help=(
            "development metrics root; data and generated directories are "
            "created below PATH (not allowed with --publish)"
        ),
    )
    parser.add_argument(
        "--force-rerun",
        action="store_true",
        help=(
            "rerun metrics phases even when compatible evidence exists; this "
            "flag takes no value and source-current executables are still reused"
        ),
    )
    parser.add_argument(
        "--consumer-mode",
        choices=("strict", "fastmath", "all"),
        default="strict",
        metavar="{strict,fastmath,all}",
        help=(
            "consumer profile; allowed values are strict, fastmath, and all "
            "(default: strict)"
        ),
    )


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--preset",
        required=True,
        metavar="PRESET",
        help="CMakePresets.json preset name",
    )
    add_workflow_arguments(parser)
    return parser.parse_args(argv)


def run_pipeline_from_args(
    root: Path,
    preset: str,
    args: argparse.Namespace,
) -> list[Path]:
    """Forward parsed public workflow arguments to one preset run."""

    return run_pipeline(
        root,
        preset,
        workflow=args.workflow,
        output_root=args.output_root,
        publish=args.publish,
        force_rerun=args.force_rerun,
        consumer_mode=args.consumer_mode,
        operations=tuple(args.operations or ()),
    )


def main(root: Path, argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        outputs = run_pipeline_from_args(root, args.preset, args)
    except PipelineError as error:
        print(f"preset metrics failed: {error}", file=sys.stderr)
        return 1
    print("generated metrics reports:", flush=True)
    for output in outputs:
        print(output, flush=True)
    return 0
