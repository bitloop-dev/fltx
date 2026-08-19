#!/usr/bin/env python3
"""Run streamed metrics applications and publish canonical metrics CSV files."""

from __future__ import annotations

import argparse
from concurrent.futures import Future, ThreadPoolExecutor
import csv
import hashlib
import json
import math
import os
import platform as host_platform
import shutil
import subprocess
import sys
import uuid
from collections import defaultdict
from collections.abc import Collection
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable, Mapping

from manifest import (
    accuracy_manifest,
    benchmark_manifest,
    enabled_implementations,
)
from source_fingerprint import source_identity, valid_fingerprint


SCHEMA_VERSION = "6"
PRECISIONS = ("f128", "f256")
CONSUMER_MODES = ("strict", "fastmath")
MINIMUM_CREDIBLE_BENCHMARK_NS = 0.001
ADAPTIVE_SAMPLE_MODES = frozenset(("small", "standard"))
DURABLE_SAMPLE_MODES = ADAPTIVE_SAMPLE_MODES | {"full"}
_ADAPTIVE_PROFILE = {
    "benchmark_trials": 7,
    "benchmark_minimum_trial_ms": 8,
    "benchmark_policy": "adaptive-v2",
    "benchmark_fast_threshold_ns": 20,
    "benchmark_slow_threshold_ns": 10_000,
    "benchmark_fast_trials": 7,
    "benchmark_ordinary_trials": 3,
    "benchmark_fast_minimum_trial_ms": 15,
    "benchmark_maximum_row_ms": 5_000,
    "benchmark_corpus_policy": "adaptive-v1",
    "accuracy_parallel": True,
}
SAMPLE_PROFILES = {
    "smoke": {
        "accuracy_samples": 12,
        "benchmark_samples": {"f128": 12, "f256": 12},
        "benchmark_trials": 3,
        "benchmark_minimum_trial_ms": 3,
    },
    "small": {
        "accuracy_samples": 2048,
        "benchmark_samples": {"f128": 4096, "f256": 2048},
        **_ADAPTIVE_PROFILE,
    },
    "standard": {
        "accuracy_samples": 4096,
        "benchmark_samples": {"f128": 8192, "f256": 4096},
        **_ADAPTIVE_PROFILE,
    },
    "full": {
        "accuracy_samples": 65536,
        "benchmark_samples": {"f128": 81920, "f256": 40960},
        "benchmark_trials": 7,
        "benchmark_minimum_trial_ms": 25,
    },
}


def _expected_benchmark_trials(
    sample_mode: str,
    profile: Mapping[str, object],
) -> int | tuple[int, ...]:
    if sample_mode in ADAPTIVE_SAMPLE_MODES:
        return (1, 3, 7)
    return int(profile["benchmark_trials"])


CANONICAL_TARGETS = {
    ("windows", "x86_64", "MSVC"): ("windows", "msvc", frozenset(("", "msvc"))),
    ("windows", "x86_64", "ClangCL"): ("windows", "clang", frozenset(("msvc",))),
    ("windows", "x86_64", "MinGW"): ("windows", "gnu", frozenset(("", "gnu"))),
    ("windows", "arm64", "MSVC"): ("windows", "msvc", frozenset(("", "msvc"))),
    ("windows", "arm64", "ClangCL"): ("windows", "clang", frozenset(("msvc",))),
    ("linux", "x86_64", "GCC"): ("linux", "gnu", frozenset(("", "gnu"))),
    ("linux", "x86_64", "Clang"): ("linux", "clang", frozenset(("", "gnu"))),
    ("linux", "arm64", "GCC"): ("linux", "gnu", frozenset(("", "gnu"))),
    ("linux", "arm64", "Clang"): ("linux", "clang", frozenset(("", "gnu"))),
    ("macos", "x86_64", "AppleClang"): ("darwin", "appleclang", frozenset(("", "gnu"))),
    ("macos", "arm64", "AppleClang"): ("darwin", "appleclang", frozenset(("", "gnu"))),
    ("wasm32", "wasm32", "Emscripten"): ("emscripten", "clang", frozenset(("", "gnu"))),
}
REQUIRED_CONFIGURATION_FIELDS = {
    "harness": {
        "qdpp",
        "tlfloat",
    },
    "consumer": {
        "fma",
        "simd",
        "fast-math",
        "simulated-consteval",
        "x86",
        "arm64",
        "tu-x86-fma",
        "has-x86-fma",
    },
    "library": {
        "fma",
        "simd",
        "fast-math-request",
        "fast-math",
        "x86",
        "arm64",
        "tu-x86-fma",
        "runtime-fma",
    },
    "library_strategy": {
        "compiled-x86-backend",
        "runtime-check",
        "msvc-guarded",
        "scalar-x86",
        "baseline-arm64",
        "runtime-path",
        "sse2",
        "neon",
        "wasm-simd",
        "f256-simd",
        "trig-simd",
    },
}
BUILD_CONFIGURATION_FIELDS = {
    "compiler-id",
    "compiler-version",
    "config",
    "system",
    "processor",
    "architecture",
    "frontend-variant",
    "optimized",
    "flags-hash",
    "source-fingerprint",
}
CONFIGURATION_FIELDS = {
    **REQUIRED_CONFIGURATION_FIELDS,
    "build": BUILD_CONFIGURATION_FIELDS,
}
RUN_CONFIGURATION_FIELDS = {
    "sample-mode",
    "samples",
    "trials",
}
HOST_FIELDS = {
    "os",
    "os-release",
    "machine",
    "processor",
    "python",
    "node",
}
EXECUTABLE_FIELDS = {
    "path",
    "sha256",
    "wasm-sha256",
}
ACCURACY_FIELDS = (
    "schema_version",
    "run_id",
    "source_revision",
    "source_fingerprint",
    "precision",
    "group",
    "operation",
    "implementation",
    "implementation_short",
    "implementation_label",
    "api",
    "domain",
    "samples",
    "seed",
    "mean_bits",
    "p01_bits",
    "worst_bits",
    "required_worst_bits",
    "margin_bits",
    "pass",
    "special_support",
    "signed_zero_support",
    "worst_input",
    "observed",
    "reference",
)
BENCHMARK_FIELDS = (
    "schema_version",
    "run_id",
    "source_revision",
    "source_fingerprint",
    "precision",
    "group",
    "operation",
    "implementation",
    "implementation_short",
    "implementation_label",
    "api",
    "samples",
    "ns_iter",
    "trials_ns",
    "iterations",
    "elapsed_ns",
    "speed_ratio",
)
CANONICAL_FIELDS = (
    "group",
    "operation",
    "precision",
    "implementation",
    "implementation_short",
    "implementation_label",
    "api",
    "samples",
    "mean_bits",
    "p01_bits",
    "worst_bits",
    "domains_passed",
    "domains_total",
    "min_margin_bits",
    "accuracy_pass",
    "special_support",
    "signed_zero_support",
    "ns_iter",
    "speed_ratio",
    "run_id",
)


class MetricsError(ValueError):
    """A metrics file is incomplete, inconsistent, or malformed."""


def consumer_mode_suffix(consumer_mode: str) -> str:
    if consumer_mode not in CONSUMER_MODES:
        raise MetricsError(f"unsupported consumer mode {consumer_mode!r}")
    return "" if consumer_mode == "strict" else "_fastmath"


def accuracy_policy_arguments(consumer_mode: str) -> tuple[str, ...]:
    if consumer_mode not in CONSUMER_MODES:
        raise MetricsError(f"unsupported consumer mode {consumer_mode!r}")
    return ("--advisory",) if consumer_mode == "fastmath" else ()


def metrics_stem(compiler: str, precision: str, consumer_mode: str) -> str:
    return f"{compiler}_{precision}{consumer_mode_suffix(consumer_mode)}"


def run_metadata_stem(compiler: str, consumer_mode: str) -> str:
    return f"{compiler}{consumer_mode_suffix(consumer_mode)}_run"


def _validate_configuration(configuration: object, source: Path | str) -> dict[str, dict[str, str]]:
    """Validate and normalize the configuration emitted by a metrics executable."""

    if not isinstance(configuration, dict):
        raise MetricsError(f"{source}: incomplete configuration metadata")
    sections = set(configuration)
    if sections != set(CONFIGURATION_FIELDS):
        raise MetricsError(f"{source}: incomplete configuration metadata")

    normalized: dict[str, dict[str, str]] = {}
    for section, required_fields in CONFIGURATION_FIELDS.items():
        values = configuration.get(section)
        if not isinstance(values, dict):
            raise MetricsError(f"{source}: invalid {section} configuration fields")
        actual_fields = set(values)
        if actual_fields != required_fields:
            missing = sorted(required_fields - actual_fields)
            unexpected = sorted(actual_fields - required_fields)
            details = []
            if missing:
                details.append(f"missing {', '.join(missing)}")
            if unexpected:
                details.append(f"unexpected {', '.join(unexpected)}")
            raise MetricsError(
                f"{source}: invalid {section} configuration fields "
                f"({'; '.join(details)}); rebuild the metrics executable"
            )
        if not all(isinstance(value, str) and value for value in values.values()):
            raise MetricsError(f"{source}: empty or non-string {section} configuration value")
        normalized[section] = dict(sorted(values.items()))

    if normalized["consumer"]["fma"] not in {"AUTO", "OFF", "ASSUME"}:
        raise MetricsError(f"{source}: invalid consumer FMA mode")
    if normalized["library"]["fma"] not in {"AUTO", "OFF", "ASSUME"}:
        raise MetricsError(f"{source}: invalid library FMA mode")
    if normalized["library"]["fast-math-request"] not in {"AUTO", "ON", "OFF"}:
        raise MetricsError(f"{source}: invalid internal fast-math request")
    if normalized["harness"]["qdpp"] not in {"on", "off"}:
        raise MetricsError(f"{source}: invalid qdpp harness state")
    if normalized["harness"]["tlfloat"] not in {"on", "off"}:
        raise MetricsError(f"{source}: invalid TLFloat harness state")
    for section, fields in (
        ("consumer", ("simd", "fast-math", "simulated-consteval")),
        ("library", ("simd", "fast-math")),
    ):
        for field in fields:
            if normalized[section][field] not in {"on", "off"}:
                raise MetricsError(f"{source}: invalid {section} {field} value")
    if normalized["library"]["runtime-fma"] not in {"available", "unavailable"}:
        raise MetricsError(f"{source}: invalid runtime FMA availability")
    if normalized["build"]["optimized"] not in {"0", "1"}:
        raise MetricsError(f"{source}: invalid build optimized flag")
    if not valid_fingerprint(normalized["build"]["source-fingerprint"]):
        raise MetricsError(f"{source}: invalid build source fingerprint")
    if not valid_fingerprint(normalized["build"]["flags-hash"]):
        raise MetricsError(f"{source}: invalid build flags hash")
    for section, values in normalized.items():
        if section in {"build", "harness"}:
            continue
        for field, value in values.items():
            if field not in {
                "fma",
                "simd",
                "fast-math",
                "fast-math-request",
                "simulated-consteval",
                "runtime-fma",
            } and value not in {"0", "1"}:
                raise MetricsError(f"{source}: invalid {section} {field} flag")
    return normalized


def require_consumer_mode(
    configuration: object,
    consumer_mode: str,
    source: Path | str,
) -> dict[str, dict[str, str]]:
    """Require evidence to match the requested consumer compiler mode."""

    normalized = _validate_configuration(configuration, source)
    if consumer_mode not in CONSUMER_MODES:
        raise MetricsError(f"unsupported consumer mode {consumer_mode!r}")
    expected = "on" if consumer_mode == "fastmath" else "off"
    observed = normalized["consumer"]["fast-math"]
    if observed != expected:
        raise MetricsError(
            f"{source}: consumer fast-math is {observed}, expected {expected} "
            f"for {consumer_mode!r} evidence"
        )
    return normalized


def _parse_configuration(lines: Iterable[str], runner_name: str) -> dict[str, dict[str, str]]:
    expected_heading = f"[fltx {runner_name}]"
    headings = 0
    sections: dict[str, dict[str, str]] = {}
    prefixes = {
        "[harness] ": "harness",
        "[consumer] ": "consumer",
        "[fltx lib] ": "library",
        "[fltx lib strategy] ": "library_strategy",
        "[build] ": "build",
    }
    for raw_line in lines:
        line = raw_line.strip()
        if line == expected_heading:
            headings += 1
        for prefix, section in prefixes.items():
            if not line.startswith(prefix):
                continue
            if section in sections:
                raise MetricsError(f"{runner_name}: duplicate {prefix.strip()} banner")
            values: dict[str, str] = {}
            for item in line[len(prefix):].split():
                if item.count("=") != 1:
                    raise MetricsError(f"{runner_name}: malformed configuration item {item!r}")
                key, value = item.split("=", 1)
                if not key or not value or key in values:
                    raise MetricsError(f"{runner_name}: malformed configuration item {item!r}")
                values[key] = value
            sections[section] = values
    if headings != 1:
        raise MetricsError(f"{runner_name}: expected one {expected_heading} banner")
    return _validate_configuration(sections, runner_name)


def _parse_run_configuration(
    lines: Iterable[str],
    runner_name: str,
) -> dict[str, str]:
    prefix = "[run] "
    matches = [line.strip()[len(prefix):] for line in lines if line.strip().startswith(prefix)]
    if len(matches) != 1:
        raise MetricsError(f"{runner_name}: expected one [run] banner")

    values: dict[str, str] = {}
    for item in matches[0].split():
        if item.count("=") != 1:
            raise MetricsError(f"{runner_name}: malformed run item {item!r}")
        key, value = item.split("=", 1)
        if not key or not value or key in values:
            raise MetricsError(f"{runner_name}: malformed run item {item!r}")
        values[key] = value
    if set(values) != RUN_CONFIGURATION_FIELDS:
        raise MetricsError(f"{runner_name}: invalid run configuration fields")
    if values["sample-mode"] not in SAMPLE_PROFILES and values["sample-mode"] != "custom":
        raise MetricsError(f"{runner_name}: invalid sample mode")
    for field in ("samples", "trials"):
        try:
            number = int(values[field])
        except ValueError as error:
            raise MetricsError(f"{runner_name}: invalid run {field}") from error
        if number < 0:
            raise MetricsError(f"{runner_name}: invalid run {field}")
    return values


def _validate_run_configuration(
    observed: Mapping[str, str],
    *,
    runner_name: str,
    sample_mode: str,
    samples: int,
    trials: int,
) -> None:
    expected = {
        "sample-mode": sample_mode,
        "samples": str(samples),
        "trials": str(trials),
    }
    if observed != expected:
        raise MetricsError(
            f"{runner_name}: run configuration mismatch "
            f"(expected {expected}, got {dict(observed)})"
        )


def _require_matching_configuration(
    expected: dict[str, dict[str, str]] | None,
    observed: dict[str, dict[str, str]],
    source: str,
) -> dict[str, dict[str, str]]:
    if expected is None:
        return observed
    if observed != expected:
        differences = []
        for section in sorted(set(expected) | set(observed)):
            if section not in expected or section not in observed:
                differences.append(
                    f"{section}: "
                    f"{'present' if section in expected else 'missing'} != "
                    f"{'present' if section in observed else 'missing'}"
                )
                continue
            for field in sorted(set(expected[section]) | set(observed[section])):
                before = expected[section].get(field, "<missing>")
                after = observed[section].get(field, "<missing>")
                if before != after:
                    differences.append(f"{section}.{field}: {before} != {after}")
        detail = "; ".join(differences) or "configuration objects differ"
        raise MetricsError(f"{source}: configuration mismatch ({detail})")
    return expected


def _require_source_fingerprint(
    configuration: Mapping[str, Mapping[str, str]],
    expected: str,
    source: Path | str,
) -> None:
    """Reject an executable built from any source tree other than ``expected``."""

    if not valid_fingerprint(expected):
        raise MetricsError(f"{source}: invalid published source fingerprint")
    observed = configuration.get("build", {}).get("source-fingerprint")
    if observed != expected:
        raise MetricsError(
            f"{source}: stale executable source fingerprint "
            f"({observed or '<missing>'} != {expected}); rebuild "
            "the requested metrics executable"
        )


def _validate_target_labels(
    platform: str,
    architecture: str,
    compiler: str,
    configuration: Mapping[str, Mapping[str, str]],
    source: Path | str,
) -> None:
    """Reject misleading output labels when the executable reports its build identity."""

    build = configuration.get("build")
    if build is None:
        return

    system = build["system"].casefold()
    reported_architecture = build["architecture"]
    compiler_id = build["compiler-id"].casefold()
    frontend_variant = build["frontend-variant"].casefold()

    expected = CANONICAL_TARGETS.get((platform, architecture, compiler))
    if expected is None:
        raise MetricsError(
            f"{source}: unsupported canonical target label "
            f"{platform}/{architecture}/{compiler}"
        )

    expected_system, expected_compiler_id, expected_frontends = expected
    if (
        system != expected_system
        or reported_architecture != architecture
        or compiler_id != expected_compiler_id
        or frontend_variant not in expected_frontends
    ):
        raise MetricsError(
            f"{source}: target label {platform}/{architecture}/{compiler} does not "
            f"match reported {build['system']}/{build['architecture']}/"
            f"{build['compiler-id']}/{build['frontend-variant']} build"
        )


def infer_canonical_target(
    configuration: Mapping[str, Mapping[str, str]],
    source: Path | str,
) -> tuple[str, str, str]:
    """Return the canonical output labels reported by a metrics runner."""

    build = configuration.get("build")
    if build is None:
        raise MetricsError(f"{source}: target inference requires build identity")

    identity = (
        build.get("system", "").casefold(),
        build.get("architecture", ""),
        build.get("compiler-id", "").casefold(),
        build.get("frontend-variant", "").casefold(),
    )
    matches = [
        target
        for target, (expected_system, expected_compiler, expected_frontends)
        in CANONICAL_TARGETS.items()
        if (
            identity[0] == expected_system
            and identity[1] == target[1]
            and identity[2] == expected_compiler
            and identity[3] in expected_frontends
        )
    ]
    if len(matches) != 1:
        raise MetricsError(
            f"{source}: unsupported canonical build identity "
            f"{build.get('system', '<missing>')}/"
            f"{build.get('architecture', '<missing>')}/"
            f"{build.get('compiler-id', '<missing>')}/"
            f"{build.get('frontend-variant', '<missing>')}"
        )
    return matches[0]


def _validate_publishable_build(
    configuration: Mapping[str, Mapping[str, str]],
    source: Path | str,
) -> None:
    """Require an identified optimized Release build for canonical publication."""

    build = configuration.get("build")
    if build is None:
        raise MetricsError(f"{source}: canonical publication requires build identity")
    if build["config"] != "Release" or build["optimized"] != "1":
        raise MetricsError(
            f"{source}: canonical publication requires an optimized Release build "
            f"(reported config={build['config']!r}, optimized={build['optimized']!r})"
        )


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as error:
        raise MetricsError(f"cannot hash executable artifact {path}: {error}") from error
    return digest.hexdigest()


def _output_hashes(
    staged_outputs: Iterable[tuple[Path, Path]],
    root: Path,
) -> dict[str, str]:
    hashes = {}
    for staged, final in staged_outputs:
        try:
            name = final.relative_to(root).as_posix()
        except ValueError as error:
            raise MetricsError(f"published output escapes target root: {final}") from error
        if name in hashes:
            raise MetricsError(f"duplicate published output path: {name}")
        hashes[name] = _sha256_file(staged)
    return dict(sorted(hashes.items()))


def _verify_output_hashes(
    root: Path,
    outputs: object,
    expected_names: Iterable[str],
    source: Path | str,
) -> None:
    expected = set(expected_names)
    if not isinstance(outputs, dict) or set(outputs) != expected:
        raise MetricsError(f"{source}: incomplete published output hash manifest")
    for name, digest in outputs.items():
        if not valid_fingerprint(digest):
            raise MetricsError(f"{source}: invalid output hash for {name}")
        path = root.joinpath(*name.split("/"))
        if _sha256_file(path) != digest:
            raise MetricsError(f"{source}: published output hash mismatch for {name}")


def _executable_identity(path: Path) -> dict[str, str]:
    try:
        resolved = path.resolve(strict=True)
    except OSError as error:
        raise MetricsError(f"cannot resolve executable artifact {path}: {error}") from error
    if not resolved.is_file():
        raise MetricsError(f"executable artifact is not a file: {resolved}")

    wasm_hash = "not-present"
    if resolved.suffix.lower() in {".js", ".mjs"}:
        wasm = resolved.with_suffix(".wasm")
        if wasm.is_file():
            wasm_hash = _sha256_file(wasm)
    return {
        "path": str(resolved),
        "sha256": _sha256_file(resolved),
        "wasm-sha256": wasm_hash,
    }


def _run_provenance(
    runners: tuple[tuple[str, Path], ...],
) -> dict[str, object]:
    executables = {
        name: _executable_identity(executable)
        for name, executable in runners
    }
    uses_node = any(
        Path(values["path"]).suffix.lower() in {".js", ".mjs"}
        for values in executables.values()
    )
    node_version = "not-used"
    if uses_node:
        node = shutil.which("node")
        if not node:
            raise MetricsError("Node.js is required to run WebAssembly metrics applications")
        try:
            node_version = subprocess.check_output(
                [node, "--version"],
                text=True,
                encoding="utf-8",
                errors="replace",
            ).strip()
        except (OSError, subprocess.CalledProcessError) as error:
            raise MetricsError(f"cannot identify Node.js runtime: {error}") from error
        if not node_version:
            raise MetricsError("Node.js returned an empty version")

    host = {
        "os": host_platform.system() or "unknown",
        "os-release": host_platform.release() or "unknown",
        "machine": host_platform.machine() or "unknown",
        "processor": host_platform.processor() or "unknown",
        "python": f"{host_platform.python_implementation()} {host_platform.python_version()}",
        "node": node_version,
    }
    return {
        "host": host,
        "executables": executables,
    }


def _inspect_runner(
    executable: Path,
    runner_name: str,
    environment: Mapping[str, str] | None = None,
) -> dict[str, dict[str, str]]:
    command = [*_runner_command(executable), "--describe"]
    try:
        process = subprocess.run(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            env=dict(environment) if environment is not None else None,
            check=False,
        )
    except OSError as error:
        raise MetricsError(
            f"cannot inspect {runner_name} executable: {error}"
        ) from error

    lines = process.stdout.splitlines(keepends=True)
    if process.returncode != 0:
        detail = process.stdout.strip()
        suffix = f": {detail}" if detail else ""
        raise MetricsError(
            f"{runner_name} executable does not support metrics preflight; "
            f"rebuild this metrics executable{suffix}"
        )
    return _parse_configuration(lines, runner_name)


def _preflight_runners(
    runners: tuple[tuple[str, Path], ...],
    *,
    fingerprint: str,
    platform: str | None,
    architecture: str | None,
    compiler: str | None,
    publishable: bool,
    environment: Mapping[str, str] | None = None,
) -> dict[str, dict[str, str]]:
    configuration: dict[str, dict[str, str]] | None = None
    for runner_name, executable in runners:
        observed = _inspect_runner(executable, runner_name, environment)
        configuration = _accept_runner_configuration(
            configuration,
            observed,
            source=f"{runner_name} preflight",
            fingerprint=fingerprint,
            platform=platform,
            architecture=architecture,
            compiler=compiler,
            publishable=publishable,
        )
    assert configuration is not None
    return configuration


def validate_runner_artifacts(
    root: Path,
    artifacts: Mapping[str, Path],
    *,
    sample_mode: str,
    consumer_mode: str = "strict",
    environment: Mapping[str, str] | None = None,
) -> tuple[str, str, str]:
    """Validate existing runners without configuring, building, or measuring."""

    if sample_mode not in SAMPLE_PROFILES:
        raise MetricsError(f"unsupported sample mode {sample_mode!r}")
    expected = {"fltx_accuracy", "fltx_benchmark"}
    if set(artifacts) != expected:
        raise MetricsError(
            "existing runner artifact set is incomplete "
            f"({', '.join(sorted(artifacts)) or 'none'})"
        )

    identity = source_identity(root.resolve())
    configuration = _preflight_runners(
        (
            ("accuracy", artifacts["fltx_accuracy"]),
            ("benchmark", artifacts["fltx_benchmark"]),
        ),
        fingerprint=identity.fingerprint,
        platform=None,
        architecture=None,
        compiler=None,
        publishable=sample_mode == "full",
        environment=environment,
    )
    require_consumer_mode(configuration, consumer_mode, "existing metrics runners")
    return infer_canonical_target(configuration, "existing metrics runners")


def _validate_provenance(
    host: object,
    executables: object,
    source: Path | str,
) -> None:
    if (
        not isinstance(host, dict)
        or set(host) != HOST_FIELDS
        or not all(isinstance(value, str) and value for value in host.values())
    ):
        raise MetricsError(f"{source}: invalid host provenance")
    if (
        not isinstance(executables, dict)
        or not executables
        or not set(executables).issubset({"accuracy", "benchmark"})
    ):
        raise MetricsError(f"{source}: invalid executable provenance")
    for name, identity in executables.items():
        if (
            not isinstance(identity, dict)
            or set(identity) != EXECUTABLE_FIELDS
            or not isinstance(identity["path"], str)
            or not identity["path"]
            or not valid_fingerprint(identity["sha256"])
            or (
                identity["wasm-sha256"] != "not-present"
                and not valid_fingerprint(identity["wasm-sha256"])
            )
        ):
            raise MetricsError(f"{source}: invalid {name} executable provenance")


def _accept_runner_configuration(
    expected: dict[str, dict[str, str]] | None,
    observed: dict[str, dict[str, str]],
    *,
    source: str,
    fingerprint: str,
    platform: str | None,
    architecture: str | None,
    compiler: str | None,
    publishable: bool,
) -> dict[str, dict[str, str]]:
    candidate = _require_matching_configuration(expected, observed, source)
    _require_source_fingerprint(candidate, fingerprint, source)
    if platform is not None and architecture is not None and compiler is not None:
        _validate_target_labels(platform, architecture, compiler, candidate, source)
    if publishable:
        _validate_publishable_build(candidate, source)
    return candidate


def _read_csv(
    path: Path,
    fields: tuple[str, ...],
    *,
    allow_partial: bool = False,
) -> list[dict[str, str]]:
    if path.name.endswith(".partial.csv") and not allow_partial:
        raise MetricsError(f"refusing partial file: {path}")
    try:
        with path.open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream)
            if tuple(reader.fieldnames or ()) != fields:
                raise MetricsError(
                    f"{path}: expected header {','.join(fields)}, "
                    f"got {','.join(reader.fieldnames or [])}"
                )
            rows = list(reader)
    except OSError as error:
        raise MetricsError(f"cannot read {path}: {error}") from error
    if not rows:
        raise MetricsError(f"{path}: contains no rows")
    if any(None in row or any(value is None for value in row.values()) for row in rows):
        raise MetricsError(f"{path}: malformed row width")
    return rows


def _float(
    row: Mapping[str, str],
    field: str,
    source: Path,
    *,
    allow_infinity: bool = False,
) -> float:
    try:
        value = float(row[field])
    except (KeyError, TypeError, ValueError) as error:
        raise MetricsError(f"{source}: invalid {field!r}: {row.get(field)!r}") from error
    if value != value or value == float("-inf") or (value == float("inf") and not allow_infinity):
        qualifier = "a number or +inf" if allow_infinity else "finite"
        raise MetricsError(f"{source}: {field!r} must be {qualifier}")
    return value


def _int(row: Mapping[str, str], field: str, source: Path) -> int:
    try:
        value = int(row[field])
    except (KeyError, TypeError, ValueError) as error:
        raise MetricsError(f"{source}: invalid {field!r}: {row.get(field)!r}") from error
    if value < 0:
        raise MetricsError(f"{source}: {field!r} must not be negative")
    return value


def _optional_float(row: Mapping[str, str], field: str, source: Path) -> float | None:
    if row.get(field, "") == "":
        return None
    return _float(row, field, source)


def _trials(
    row: Mapping[str, str],
    field: str,
    source: Path,
    *,
    optional: bool = False,
) -> list[float] | None:
    raw = row.get(field, "")
    if raw == "" and optional:
        return None
    if not raw or any(not item for item in raw.split("|")):
        raise MetricsError(f"{source}: invalid {field!r} trial list")
    values = []
    for item in raw.split("|"):
        try:
            value = float(item)
        except ValueError as error:
            raise MetricsError(f"{source}: invalid {field!r} trial {item!r}") from error
        if not math.isfinite(value) or value <= 0:
            raise MetricsError(f"{source}: {field!r} trials must be finite and positive")
        values.append(value)
    return values


def _benchmark_values(
    row: Mapping[str, str],
    source: Path,
    *,
    expected_trials: int | Collection[int] | None = None,
) -> tuple[float, float | None]:
    time = _float(row, "ns_iter", source)
    trials = _trials(row, "trials_ns", source)
    assert trials is not None
    valid_trial_count = (
        expected_trials is None
        or (
            len(trials) == expected_trials
            if isinstance(expected_trials, int)
            else len(trials) in expected_trials
        )
    )
    if time < MINIMUM_CREDIBLE_BENCHMARK_NS:
        raise MetricsError(
            f"{source}: implausibly small benchmark timing; "
            "the workload may have been optimized away"
        )
    if not valid_trial_count:
        raise MetricsError(f"{source}: invalid benchmark trials")
    if not math.isclose(
        time,
        sorted(trials)[len(trials) // 2],
        rel_tol=2e-5,
        abs_tol=2e-6,
    ):
        raise MetricsError(f"{source}: median does not match its trials")
    if _int(row, "iterations", source) <= 0:
        raise MetricsError(f"{source}: invalid iteration count")
    if _float(row, "elapsed_ns", source) <= 0:
        raise MetricsError(f"{source}: invalid elapsed time")
    ratio = _optional_float(row, "speed_ratio", source)
    if row["implementation"] == "fltx":
        if ratio is not None:
            raise MetricsError(f"{source}: FLTX speed ratio must be empty")
    elif ratio is None or ratio <= 0:
        raise MetricsError(f"{source}: comparison speed ratio must be positive")
    return time, ratio


def _validate_identity(
    rows: Iterable[Mapping[str, str]],
    path: Path,
    *,
    run_id: str,
    revision: str,
    fingerprint: str,
    precision: str,
) -> None:
    if not valid_fingerprint(fingerprint):
        raise MetricsError(f"{path}: invalid expected source fingerprint")
    seen: set[tuple[str, ...]] = set()
    accuracy = tuple(rows)
    for row in accuracy:
        if row["schema_version"] != SCHEMA_VERSION:
            raise MetricsError(f"{path}: unsupported schema version {row['schema_version']!r}")
        if row["run_id"] != run_id or row["source_revision"] != revision:
            raise MetricsError(f"{path}: mixed run ID or source revision")
        if row["source_fingerprint"] != fingerprint:
            raise MetricsError(f"{path}: mixed or stale source fingerprint")
        if row["precision"] != precision:
            raise MetricsError(f"{path}: expected precision {precision}, got {row['precision']!r}")
        if (
            not row["group"] or not row["operation"] or not row["implementation"]
            or not row["implementation_short"] or not row["implementation_label"]
            or not row["api"] or ("domain" in row and not row["domain"])
        ):
            raise MetricsError(f"{path}: result identity fields must not be empty")
        key = (
            row["group"], row["operation"], row["implementation"],
            row.get("domain", ""),
        )
        if key in seen:
            raise MetricsError(f"{path}: duplicate result {key}")
        seen.add(key)
        if _int(row, "samples", path) == 0:
            raise MetricsError(f"{path}: samples must be greater than zero")
        if "seed" in row:
            _int(row, "seed", path)


def _validate_completeness(
    accuracy_rows: Iterable[Mapping[str, str]],
    benchmark_rows: Iterable[Mapping[str, str]],
    *,
    precision: str,
    accuracy_path: Path,
    benchmark_path: Path,
    qdpp_enabled: bool,
    tlfloat_enabled: bool,
    consumer_mode: str = "strict",
) -> None:
    for source, expected, rows, domain_field in (
        (
            accuracy_path,
            accuracy_manifest(
                precision, qdpp=qdpp_enabled, tlfloat=tlfloat_enabled,
            ),
            accuracy_rows,
            True,
        ),
        (
            benchmark_path,
            benchmark_manifest(
                precision, qdpp=qdpp_enabled, tlfloat=tlfloat_enabled,
            ),
            benchmark_rows,
            False,
        ),
    ):
        if consumer_mode == "fastmath":
            expected = {"fltx": expected["fltx"]}
        observed: dict[str, set[tuple[str, ...]]] = defaultdict(set)
        for row in rows:
            key = (row["group"], row["operation"])
            if domain_field:
                key += (row["domain"],)
            observed[row["implementation"]].add(key)
        if set(observed) != set(expected):
            raise MetricsError(
                f"{source}: implementation manifest mismatch "
                f"(expected {sorted(expected)}, got {sorted(observed)})"
            )
        for implementation, expected_rows in expected.items():
            missing = sorted(expected_rows - observed[implementation])
            extra = sorted(observed[implementation] - expected_rows)
            if missing or extra:
                raise MetricsError(
                    f"{source}: incomplete {implementation} manifest "
                    f"(missing {missing[:5]}, unexpected {extra[:5]})"
                )


def _validate_implementations(
    benchmark_rows: Iterable[Mapping[str, str]],
    *,
    precision: str,
    benchmark_path: Path,
    qdpp_enabled: bool,
    tlfloat_enabled: bool,
    consumer_mode: str = "strict",
) -> None:
    expected = {
        item.id: item
        for item in enabled_implementations(
            precision, qdpp=qdpp_enabled, tlfloat=tlfloat_enabled,
        )
    }
    if consumer_mode == "fastmath":
        expected = {"fltx": expected["fltx"]}
    observed: dict[str, tuple[str, str]] = {}
    fltx_times: dict[tuple[str, str], float] = {}
    parsed: list[tuple[Mapping[str, str], float, float | None]] = []
    for row in benchmark_rows:
        implementation = row["implementation"]
        if implementation not in expected:
            raise MetricsError(
                f"{benchmark_path}: unexpected implementation {implementation!r}"
            )
        identity = (row["implementation_short"], row["implementation_label"])
        previous = observed.setdefault(implementation, identity)
        if previous != identity:
            raise MetricsError(
                f"{benchmark_path}: inconsistent identity for {implementation}"
            )
        expected_identity = expected[implementation]
        if identity != (expected_identity.short_label, expected_identity.label):
            raise MetricsError(
                f"{benchmark_path}: incorrect label for {implementation}"
            )
        time, ratio = _benchmark_values(row, benchmark_path)
        parsed.append((row, time, ratio))
        if implementation == "fltx":
            fltx_times[(row["group"], row["operation"])] = time
    for row, time, ratio in parsed:
        if row["implementation"] == "fltx":
            continue
        key = (row["group"], row["operation"])
        if key not in fltx_times or ratio is None or not math.isclose(
            ratio, time / fltx_times[key], rel_tol=2e-5, abs_tol=2e-6,
        ):
            raise MetricsError(
                f"{benchmark_path}: inconsistent speed ratio for "
                f"{row['implementation']} {key}"
            )


def _accuracy_values(
    row: Mapping[str, str],
    source: Path,
) -> tuple[float, float, float, float | None, bool | None, bool]:
    mean, p01, worst = (
        _float(row, field, source, allow_infinity=True)
        for field in ("mean_bits", "p01_bits", "worst_bits")
    )
    exact = math.isinf(mean)
    if exact and not (math.isinf(p01) and math.isinf(worst)):
        raise MetricsError(f"{source}: inconsistent exact metrics for {row['operation']}")
    if not exact and math.isinf(worst):
        raise MetricsError(f"{source}: non-exact result has exact worst bits")
    if min(mean, p01, worst) < 0 or p01 < worst or mean < worst:
        raise MetricsError(f"{source}: invalid accuracy ordering for {row['operation']}")
    if row["implementation"] not in {"fltx", "native"}:
        if any(row[field] for field in ("required_worst_bits", "margin_bits", "pass")):
            raise MetricsError(f"{source}: competitor threshold fields must be empty")
        return mean, p01, worst, None, None, exact
    required = _float(row, "required_worst_bits", source)
    margin = _float(row, "margin_bits", source, allow_infinity=True)
    if row["pass"] not in {"yes", "no"}:
        raise MetricsError(f"{source}: pass must be 'yes' or 'no'")
    if exact != math.isinf(margin):
        raise MetricsError(f"{source}: inconsistent exact margin for {row['operation']}")
    expected_margin = worst - required
    if math.isfinite(expected_margin) and not math.isclose(
        margin, expected_margin, rel_tol=1e-8, abs_tol=1e-5,
    ):
        raise MetricsError(f"{source}: inconsistent margin for {row['operation']}")
    passed = row["pass"] == "yes"
    if passed != (margin >= 0):
        raise MetricsError(f"{source}: inconsistent pass flag for {row['operation']}")
    return mean, p01, worst, margin, passed, exact


def _accuracy_summary(
    detail: list[dict[str, str]],
    source: Path,
    thresholds: Mapping[str, float],
) -> dict[str, object]:
    counts = [_int(row, "samples", source) for row in detail]
    total = sum(counts)
    precision = detail[0]["precision"]
    if total <= 0:
        raise MetricsError(f"{source}: invalid accuracy samples or precision")

    implementation_id = detail[0]["implementation"]
    try:
        implementation = next(
            item
            for item in enabled_implementations(
                precision, qdpp=True, tlfloat=True,
            )
            if item.id == implementation_id
        )
    except (KeyError, StopIteration) as error:
        raise MetricsError(
            f"{source}: unknown implementation {implementation_id!r}"
        ) from error

    values = [_accuracy_values(row, source) for row in detail]
    means, p01s, worsts, margins, passes, exact = zip(*values)
    all_exact = all(exact)
    pooled = [
        implementation.nominal_bits + 32.0 if is_exact else mean
        for mean, is_exact in zip(means, exact)
    ]
    domains = {row["domain"] for row in detail}
    if not domains.issubset(thresholds):
        raise MetricsError(
            f"{source}: accuracy domain lacks an FLTX threshold for "
            f"{detail[0]['implementation']} {detail[0]['operation']}"
        )
    evaluated_margins: list[float] = []
    evaluated_passes: list[bool] = []
    for row, worst, recorded_margin, recorded_pass in zip(
        detail, worsts, margins, passes,
    ):
        margin = worst - thresholds[row["domain"]]
        passed = margin >= 0
        if row["implementation"] == "fltx":
            if recorded_margin is None or recorded_pass is None:
                raise MetricsError(f"{source}: FLTX threshold result is missing")
            if math.isfinite(margin) and not math.isclose(
                recorded_margin, margin, rel_tol=1e-8, abs_tol=1e-5,
            ):
                raise MetricsError(
                    f"{source}: FLTX threshold result disagrees with aggregation"
                )
            if recorded_pass != passed:
                raise MetricsError(f"{source}: FLTX pass result disagrees with aggregation")
        evaluated_margins.append(margin)
        evaluated_passes.append(passed)

    special = {row["special_support"] for row in detail}
    if not special.issubset({"Both", "Inf", "NaN", "No", "-"}):
        raise MetricsError(f"{source}: malformed special_support category")
    if len(special) != 1:
        raise MetricsError(
            f"{source}: inconsistent special_support for "
            f"{detail[0]['implementation']} {detail[0]['operation']}"
        )
    signed_zero = {row["signed_zero_support"] for row in detail}
    if not signed_zero.issubset({"yes", "no", "-"}):
        raise MetricsError(f"{source}: malformed signed_zero_support category")
    if len(signed_zero) != 1:
        raise MetricsError(
            f"{source}: inconsistent signed_zero_support for "
            f"{detail[0]['implementation']} {detail[0]['operation']}"
        )
    return {
        "samples": total,
        "mean_bits": (
            math.inf
            if all_exact
            else sum(value * count for value, count in zip(pooled, counts)) / total
        ),
        "p01_bits": min(p01s),
        "worst_bits": min(worsts),
        "domains_passed": sum(evaluated_passes),
        "domains_total": len(evaluated_passes),
        "min_margin_bits": min(evaluated_margins),
        "accuracy_pass": "yes" if all(evaluated_passes) else "no",
        "special_support": special.pop(),
        "signed_zero_support": signed_zero.pop(),
    }


def _benchmark_summary(
    row: dict[str, str],
    source: Path,
    expected_trials: int | Collection[int] | None,
) -> dict[str, object]:
    time, ratio = _benchmark_values(
        row, source, expected_trials=expected_trials,
    )
    return {
        "ns_iter": time,
        "speed_ratio": ratio if ratio is not None else "",
    }


def aggregate(
    accuracy_rows: list[dict[str, str]],
    benchmark_rows: list[dict[str, str]],
    *,
    accuracy_path: Path,
    benchmark_path: Path,
    run_id: str,
    expected_trials: int | Collection[int] | None = None,
) -> list[dict[str, object]]:
    """Merge detailed rows into one conservative summary row per operation."""

    accuracy_by_key: dict[tuple[str, str, str, str], list[dict[str, str]]] = defaultdict(list)
    benchmark_by_key: dict[tuple[str, str, str, str], dict[str, str]] = {}
    thresholds: dict[tuple[str, str, str, str], float] = {}
    for row in accuracy_rows:
        accuracy_by_key[
            (row["group"], row["operation"], row["precision"], row["implementation"])
        ].append(row)
        if row["implementation"] == "fltx":
            threshold_key = (
                row["group"], row["operation"], row["precision"], row["domain"],
            )
            required = _float(row, "required_worst_bits", accuracy_path)
            previous = thresholds.setdefault(threshold_key, required)
            if previous != required:
                raise MetricsError(
                    f"{accuracy_path}: inconsistent FLTX threshold {threshold_key}"
                )
    for row in benchmark_rows:
        key = (
            row["group"], row["operation"], row["precision"], row["implementation"],
        )
        if key in benchmark_by_key:
            raise MetricsError(f"{benchmark_path}: duplicate benchmark result {key}")
        benchmark_by_key[key] = row

    results: list[dict[str, object]] = []
    keys = sorted(set(accuracy_by_key) | set(benchmark_by_key))
    for group, operation, precision, implementation in keys:
        key = (group, operation, precision, implementation)
        detail = accuracy_by_key.get(key, [])
        benchmark = benchmark_by_key.get(key)
        identity = detail[0] if detail else benchmark
        assert identity is not None
        operation_thresholds = {
            domain: required
            for (threshold_group, threshold_operation, threshold_precision, domain), required
            in thresholds.items()
            if (threshold_group, threshold_operation, threshold_precision) ==
               (group, operation, precision)
        }
        accuracy_values = _accuracy_summary(
            detail, accuracy_path, operation_thresholds,
        ) if detail else dict.fromkeys(
            ("samples", "mean_bits", "p01_bits", "worst_bits", "domains_passed",
             "domains_total", "min_margin_bits", "accuracy_pass",
             "special_support", "signed_zero_support"),
            "",
        )
        if not detail:
            accuracy_values["special_support"] = "-"
            accuracy_values["signed_zero_support"] = "-"
        benchmark_values = (
            _benchmark_summary(benchmark, benchmark_path, expected_trials)
            if benchmark
            else dict.fromkeys(
                ("ns_iter", "speed_ratio"), ""
            )
        )
        results.append(
            {
                "group": group,
                "operation": operation,
                "precision": precision,
                "implementation": implementation,
                "implementation_short": identity["implementation_short"],
                "implementation_label": identity["implementation_label"],
                "api": identity["api"],
                **accuracy_values,
                **benchmark_values,
                "run_id": run_id,
            }
        )
    return results


def _format(value: object) -> object:
    if isinstance(value, float):
        return f"{value:.12g}"
    return value


def _atomic_csv(path: Path, rows: list[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    partial = path.with_suffix(".partial.csv")
    with partial.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=CANONICAL_FIELDS, lineterminator="\n")
        writer.writeheader()
        writer.writerows({key: _format(row[key]) for key in CANONICAL_FIELDS} for row in rows)
        stream.flush()
        os.fsync(stream.fileno())
    partial.replace(path)


def merge_precision(
    accuracy_path: Path,
    benchmark_path: Path,
    output_path: Path,
    *,
    run_id: str,
    revision: str,
    fingerprint: str,
    precision: str,
    qdpp_enabled: bool,
    tlfloat_enabled: bool,
    consumer_mode: str = "strict",
    require_complete: bool = False,
    expected_trials: int | Collection[int] | None = None,
) -> None:
    accuracy = _read_csv(accuracy_path, ACCURACY_FIELDS)
    benchmark = _read_csv(benchmark_path, BENCHMARK_FIELDS)
    _validate_identity(
        accuracy, accuracy_path,
        run_id=run_id, revision=revision, fingerprint=fingerprint,
        precision=precision,
    )
    _validate_identity(
        benchmark, benchmark_path,
        run_id=run_id, revision=revision, fingerprint=fingerprint,
        precision=precision,
    )
    if require_complete:
        _validate_completeness(
            accuracy,
            benchmark,
            precision=precision,
            accuracy_path=accuracy_path,
            benchmark_path=benchmark_path,
            qdpp_enabled=qdpp_enabled,
            tlfloat_enabled=tlfloat_enabled,
            consumer_mode=consumer_mode,
        )
    _validate_implementations(
        benchmark,
        precision=precision,
        benchmark_path=benchmark_path,
        qdpp_enabled=qdpp_enabled,
        tlfloat_enabled=tlfloat_enabled,
        consumer_mode=consumer_mode,
    )
    rows = aggregate(
        accuracy,
        benchmark,
        accuracy_path=accuracy_path,
        benchmark_path=benchmark_path,
        run_id=run_id,
        expected_trials=expected_trials,
    )
    _atomic_csv(output_path, rows)


def _run(
    command: list[str],
    runner_name: str,
    *,
    sample_mode: str,
    samples: int,
    trials: int,
) -> tuple[dict[str, dict[str, str]], int]:
    print("+", subprocess.list2cmdline(command), flush=True)
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        bufsize=1,
    )
    lines = []
    assert process.stdout is not None
    for line in process.stdout:
        lines.append(line)
        sys.stdout.write(line)
        sys.stdout.flush()
    return_code = process.wait()
    run_configuration = _parse_run_configuration(lines, runner_name)
    _validate_run_configuration(
        run_configuration,
        runner_name=runner_name,
        sample_mode=sample_mode,
        samples=samples,
        trials=trials,
    )
    return _parse_configuration(lines, runner_name), return_code


def _runner_command(executable: Path) -> list[str]:
    if executable.suffix.lower() in {".js", ".mjs"}:
        node = shutil.which("node")
        if not node:
            raise MetricsError("Node.js is required to run a WebAssembly metrics application")
        return [node, str(executable)]
    return [str(executable)]


def _safe_component(value: str, label: str) -> str:
    if (
        not value
        or value in {".", ".."}
        or any(not (character.isalnum() or character in "._-") for character in value)
    ):
        raise MetricsError(f"{label} must be a simple file-name component")
    return value


def _write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    writing = path.with_suffix(path.suffix + ".writing")
    with writing.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(value, indent=2, sort_keys=True) + "\n")
        stream.flush()
        os.fsync(stream.fileno())
    writing.replace(path)


def _result_handoff(
    *,
    run_id: str,
    sample_mode: str,
    consumer_mode: str,
    platform: str,
    architecture: str,
    compiler: str,
    input_root: Path,
    metadata: Path,
) -> dict[str, object]:
    return {
        "schema_version": 1,
        "run_id": run_id,
        "sample_mode": sample_mode,
        "consumer_mode": consumer_mode,
        "platform": platform,
        "architecture": architecture,
        "compiler": compiler,
        "target": f"{platform}/{architecture}/{compiler}",
        "input_root": str(input_root.resolve()),
        "metadata": str(metadata.resolve()),
    }


def _read_metadata(path: Path) -> dict[str, object]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise MetricsError(f"cannot read reusable metadata {path}: {error}") from error
    if not isinstance(value, dict):
        raise MetricsError(f"{path}: reusable metadata is not an object")
    return value


def _benchmark_host_identity(host: object) -> dict[str, object]:
    if not isinstance(host, dict):
        return {}
    # Python renders/orchestrates the run but is not in the timed C++/Node path.
    return {
        field: host.get(field)
        for field in ("os", "os-release", "machine", "processor", "node")
    }


def _validate_reusable_candidate(
    metadata_path: Path,
    *,
    sample_mode: str,
    consumer_mode: str = "strict",
    platform: str,
    architecture: str,
    compiler: str,
    fingerprint: str,
    configuration: Mapping[str, Mapping[str, str]],
    host: object,
) -> dict[str, object]:
    """Return a handoff for complete evidence matching the freshly built runners."""

    metadata = _read_metadata(metadata_path)
    expected_profile = SAMPLE_PROFILES[sample_mode]
    required = {
        "schema_version": SCHEMA_VERSION,
        "status": "complete",
        "source_fingerprint": fingerprint,
        "platform": platform,
        "architecture": architecture,
        "compiler": compiler,
        "sample_mode": sample_mode,
        "precisions": list(PRECISIONS),
        "phases_requested": ["accuracy", "benchmark"],
    }
    for field, expected in required.items():
        if metadata.get(field) != expected:
            raise MetricsError(
                f"{metadata_path}: reusable {field} mismatch "
                f"({metadata.get(field)!r} != {expected!r})"
            )
    observed_consumer_mode = metadata.get("consumer_mode", "strict")
    if observed_consumer_mode != consumer_mode:
        raise MetricsError(
            f"{metadata_path}: reusable consumer_mode mismatch "
            f"({observed_consumer_mode!r} != {consumer_mode!r})"
        )
    for field, expected in expected_profile.items():
        if metadata.get(field) != expected:
            raise MetricsError(f"{metadata_path}: reusable sample profile changed")
    if metadata.get("configuration") != configuration:
        raise MetricsError(f"{metadata_path}: reusable build configuration changed")
    if _benchmark_host_identity(metadata.get("host")) != _benchmark_host_identity(host):
        raise MetricsError(f"{metadata_path}: reusable host identity changed")

    run_id = metadata.get("run_id")
    revision = metadata.get("source_revision")
    if not isinstance(run_id, str) or not run_id:
        raise MetricsError(f"{metadata_path}: reusable run ID is missing")
    if not isinstance(revision, str) or not revision:
        raise MetricsError(f"{metadata_path}: reusable source revision is missing")

    phases = metadata.get("phases")
    expected_phases = {
        f"{precision}.{runner}"
        for precision in PRECISIONS
        for runner in ("accuracy", "benchmark")
    }
    if (
        not isinstance(phases, dict)
        or set(phases) != expected_phases
        or any(
            not isinstance(phase, dict) or phase.get("status") != "passed"
            for phase in phases.values()
        )
    ):
        raise MetricsError(f"{metadata_path}: reusable phases are incomplete")

    input_root = metadata_path.parents[3]
    target_root = input_root / platform / architecture
    expected_outputs = {
        f"{metrics_stem(compiler, precision, consumer_mode)}.csv"
        for precision in PRECISIONS
    } | {
        f"detail/{metrics_stem(compiler, precision, consumer_mode)}_{runner}.csv"
        for precision in PRECISIONS
        for runner in ("accuracy", "benchmark")
    }
    _verify_output_hashes(
        target_root,
        metadata.get("outputs"),
        expected_outputs,
        metadata_path,
    )

    normalized = require_consumer_mode(
        configuration,
        consumer_mode,
        "current runners",
    )
    qdpp_enabled = normalized["harness"]["qdpp"] == "on"
    tlfloat_enabled = normalized["harness"]["tlfloat"] == "on"
    for precision in PRECISIONS:
        stem = metrics_stem(compiler, precision, consumer_mode)
        accuracy_path = target_root / "detail" / f"{stem}_accuracy.csv"
        benchmark_path = target_root / "detail" / f"{stem}_benchmark.csv"
        canonical_path = target_root / f"{stem}.csv"
        accuracy_rows = _read_csv(accuracy_path, ACCURACY_FIELDS)
        benchmark_rows = _read_csv(benchmark_path, BENCHMARK_FIELDS)
        canonical_rows = _read_csv(canonical_path, CANONICAL_FIELDS)
        for rows, path in (
            (accuracy_rows, accuracy_path),
            (benchmark_rows, benchmark_path),
        ):
            _validate_identity(
                rows,
                path,
                run_id=run_id,
                revision=revision,
                fingerprint=fingerprint,
                precision=precision,
            )
        _validate_completeness(
            accuracy_rows,
            benchmark_rows,
            precision=precision,
            accuracy_path=accuracy_path,
            benchmark_path=benchmark_path,
            qdpp_enabled=qdpp_enabled,
            tlfloat_enabled=tlfloat_enabled,
            consumer_mode=consumer_mode,
        )
        _validate_implementations(
            benchmark_rows,
            precision=precision,
            benchmark_path=benchmark_path,
            qdpp_enabled=qdpp_enabled,
            tlfloat_enabled=tlfloat_enabled,
            consumer_mode=consumer_mode,
        )
        expected_rows = aggregate(
            accuracy_rows,
            benchmark_rows,
            accuracy_path=accuracy_path,
            benchmark_path=benchmark_path,
            run_id=run_id,
            expected_trials=_expected_benchmark_trials(
                sample_mode, expected_profile,
            ),
        )
        expected_summary = {
            (str(row["group"]), str(row["operation"]), str(row["implementation"])): {
                field: str(_format(row[field]))
                for field in CANONICAL_FIELDS
            }
            for row in expected_rows
        }
        observed_summary = {
            (row["group"], row["operation"], row["implementation"]): row
            for row in canonical_rows
        }
        if len(observed_summary) != len(canonical_rows) or observed_summary != expected_summary:
            raise MetricsError(f"{canonical_path}: reusable canonical summary is inconsistent")

    return _result_handoff(
        run_id=run_id,
        sample_mode=sample_mode,
        consumer_mode=consumer_mode,
        platform=platform,
        architecture=architecture,
        compiler=compiler,
        input_root=input_root,
        metadata=metadata_path,
    )


def _find_reusable_run(
    root: Path,
    output_root: Path,
    *,
    sample_mode: str,
    consumer_mode: str,
    platform: str,
    architecture: str,
    compiler: str,
    fingerprint: str,
    configuration: Mapping[str, Mapping[str, str]],
    host: object,
) -> tuple[dict[str, object] | None, str]:
    if sample_mode in DURABLE_SAMPLE_MODES:
        candidates = [
            output_root
            / platform
            / architecture
            / "detail"
            / f"{run_metadata_stem(compiler, consumer_mode)}.json"
        ]
    else:
        candidates = list(
            (root / "build" / "metrics" / "runs").glob(
                f"*/{platform}/{architecture}/detail/"
                f"{run_metadata_stem(compiler, consumer_mode)}.json"
            )
        )
        candidates.sort(
            key=lambda path: path.stat().st_mtime if path.exists() else 0,
            reverse=True,
        )
    if not candidates:
        return None, "no previous complete evidence exists"

    rejection = "no compatible evidence exists"
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


@contextmanager
def _publication_lock(path: Path):
    """Hold a non-blocking OS lock while replacing one target's result set."""

    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        stream = path.open("a+b")
    except OSError as error:
        raise MetricsError(f"cannot open publication lock {path}: {error}") from error

    try:
        stream.seek(0, os.SEEK_END)
        if stream.tell() == 0:
            stream.write(b"\0")
            stream.flush()
        stream.seek(0)

        try:
            if os.name == "nt":
                import msvcrt

                msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
            else:
                import fcntl

                fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError as error:
            raise MetricsError(
                f"another metrics publication is already active for {path.parent}"
            ) from error

        yield
    finally:
        stream.close()


def _publish_transaction(
    staged_outputs: Iterable[tuple[Path, Path]],
    metadata_partial: Path,
    final_metadata: Path,
    *,
    replace=os.replace,
) -> None:
    """Publish a complete result set and roll back atomically on failure."""

    outputs = list(staged_outputs)
    destinations = [final for _, final in outputs]
    if len(set(destinations)) != len(destinations) or final_metadata in destinations:
        raise MetricsError("publish destinations must be unique")
    sources = [staged for staged, _ in outputs] + [metadata_partial]
    if len(set(sources)) != len(sources) or any(not source.is_file() for source in sources):
        raise MetricsError("publish sources must be unique complete files")

    backup_directory = metadata_partial.parent / ".rollback"
    backup_directory.mkdir(parents=False, exist_ok=False)
    publications = [*outputs, (metadata_partial, final_metadata)]
    backups: list[tuple[Path, Path]] = []
    installed: list[tuple[Path, Path]] = []

    try:
        # Removing the published commit marker first makes an in-progress
        # replacement unreadable as a complete run. The staged marker is
        # always installed last.
        ordered_destinations = [final_metadata, *destinations]
        for index, final in enumerate(ordered_destinations):
            if not final.exists():
                continue
            backup = backup_directory / f"{index:04d}.previous"
            replace(final, backup)
            backups.append((backup, final))

        for staged, final in publications:
            final.parent.mkdir(parents=True, exist_ok=True)
            replace(staged, final)
            installed.append((final, staged))
    except (OSError, MetricsError) as publish_error:
        rollback_errors: list[str] = []
        for final, staged in reversed(installed):
            try:
                if final.exists():
                    os.replace(final, staged)
            except OSError as error:
                rollback_errors.append(f"{final}: {error}")
        for backup, final in reversed(backups):
            try:
                if backup.exists():
                    final.parent.mkdir(parents=True, exist_ok=True)
                    os.replace(backup, final)
            except OSError as error:
                rollback_errors.append(f"{final}: {error}")
        try:
            backup_directory.rmdir()
        except OSError:
            pass
        if rollback_errors:
            raise MetricsError(
                f"publish failed: {publish_error}; rollback failed: "
                + "; ".join(rollback_errors)
            ) from publish_error
        raise

    # The metadata marker commits the set. Backup cleanup failure is harmless
    # and leaves the files only under .staging.
    for backup, _ in backups:
        try:
            backup.unlink()
        except OSError:
            pass
    try:
        backup_directory.rmdir()
    except OSError:
        pass


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--accuracy", type=Path, help="fltx_accuracy executable")
    parser.add_argument("--benchmark", type=Path, help="fltx_benchmark executable")
    parser.add_argument("--output-root", type=Path)
    parser.add_argument(
        "--platform",
        help="Output folder, for example windows or wasm32",
    )
    parser.add_argument("--architecture", help="Target architecture, for example x86_64")
    parser.add_argument("--compiler", help="File prefix, for example MSVC or Emscripten")
    parser.add_argument("--precision", action="append", choices=PRECISIONS, dest="precisions")
    parser.add_argument(
        "--sample-mode",
        choices=tuple(SAMPLE_PROFILES),
        default="full",
    )
    parser.add_argument(
        "--consumer-mode",
        choices=CONSUMER_MODES,
        default="strict",
        help="consumer compiler mode represented by these runners",
    )
    parser.add_argument("--result-file", type=Path, help=argparse.SUPPRESS)
    parser.add_argument("--reuse-compatible", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--canonical-publication", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    if args.accuracy is None and args.benchmark is None:
        parser.error("at least one of --accuracy or --benchmark is required")
    explicit_identity = (args.platform, args.architecture, args.compiler)
    if any(value is not None for value in explicit_identity) and not all(
        value is not None for value in explicit_identity
    ):
        parser.error(
            "--platform, --architecture, and --compiler must be supplied together"
        )
    return args


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    platform: str | None = None
    architecture: str | None = None
    compiler: str | None = None
    if (
        args.platform is not None
        and args.architecture is not None
        and args.compiler is not None
    ):
        try:
            platform = _safe_component(args.platform, "platform")
            architecture = _safe_component(args.architecture, "architecture")
            compiler = _safe_component(args.compiler, "compiler")
            if (platform, architecture, compiler) not in CANONICAL_TARGETS:
                raise MetricsError(
                    "unsupported canonical target label "
                    f"{platform}/{architecture}/{compiler}"
                )
        except MetricsError as error:
            print(f"metrics failed: {error}", file=sys.stderr)
            return 1
    precisions = tuple(dict.fromkeys(args.precisions or PRECISIONS))
    profile = SAMPLE_PROFILES[args.sample_mode]
    runners = tuple(
        (name, executable)
        for name, executable in (
            ("accuracy", args.accuracy),
            ("benchmark", args.benchmark),
        )
        if executable is not None
    )
    runner_names = {name for name, _ in runners}
    root = Path(__file__).resolve().parents[3]
    identity = source_identity(root)
    revision = identity.revision
    fingerprint = identity.fingerprint
    run_id = uuid.uuid4().hex
    complete_run = (
        runner_names == {"accuracy", "benchmark"}
        and args.sample_mode == "full"
        and set(precisions) == set(PRECISIONS)
    )
    complete_suite = (
        runner_names == {"accuracy", "benchmark"}
        and set(precisions) == set(PRECISIONS)
    )
    durable_run = complete_suite and args.sample_mode in DURABLE_SAMPLE_MODES
    canonical_output_root = (root / "validation" / "metrics" / "data").resolve()
    output_root = (
        args.output_root.resolve()
        if args.output_root is not None
        else root / "build" / "metrics" / "data"
    )
    if output_root == canonical_output_root and not args.canonical_publication:
        print(
            "metrics failed: canonical data may only be written by "
            "run_preset_metrics.py --release",
            file=sys.stderr,
        )
        return 1
    if args.canonical_publication and (
        args.sample_mode != "full" or output_root != canonical_output_root
    ):
        print(
            "metrics failed: canonical publication requires the full profile "
            "and canonical output root",
            file=sys.stderr,
        )
        return 1
    if complete_run and revision == "unknown":
        print(
            "metrics failed: a full published run requires a known source revision",
            file=sys.stderr,
        )
        return 1
    try:
        provenance = _run_provenance(runners)
        configuration = _preflight_runners(
            runners,
            fingerprint=fingerprint,
            platform=platform,
            architecture=architecture,
            compiler=compiler,
            publishable=complete_run,
        )
        configuration = require_consumer_mode(
            configuration,
            args.consumer_mode,
            "metrics runner preflight",
        )
        if platform is None or architecture is None or compiler is None:
            platform, architecture, compiler = infer_canonical_target(
                configuration,
                "metrics runner preflight",
            )
    except MetricsError as error:
        print(f"metrics failed: {error}", file=sys.stderr)
        return 1
    assert platform is not None and architecture is not None and compiler is not None
    if args.reuse_compatible and complete_suite:
        reusable, rejection = _find_reusable_run(
            root,
            output_root,
            sample_mode=args.sample_mode,
            consumer_mode=args.consumer_mode,
            platform=platform,
            architecture=architecture,
            compiler=compiler,
            fingerprint=fingerprint,
            configuration=configuration,
            host=provenance["host"],
        )
        if reusable is not None:
            if args.result_file is not None:
                try:
                    _write_json(args.result_file.resolve(), reusable)
                except OSError as error:
                    print(
                        f"metrics failed: cannot write result file "
                        f"{args.result_file}: {error}",
                        file=sys.stderr,
                    )
                    return 1
            print(
                f"reusing compatible {args.sample_mode} metrics run "
                f"{reusable['run_id']} for {reusable['target']}; "
                "runner phases skipped",
                flush=True,
            )
            return 0
        print(
            f"no reusable {args.sample_mode} metrics evidence: {rejection}; "
            "running metrics",
            flush=True,
        )

    target = (
        output_root / platform
        / architecture
        if durable_run
        else root / "build" / "metrics" / "runs" / run_id / platform / architecture
    )
    detail = target / "detail"
    staging = detail / ".staging" / run_id
    metadata = {
        "schema_version": SCHEMA_VERSION,
        "run_id": run_id,
        "source_revision": revision,
        "source_fingerprint": fingerprint,
        "platform": platform,
        "architecture": architecture,
        "compiler": compiler,
        "precisions": list(precisions),
        "implementations": {
            precision: [
                {
                    "id": implementation.id,
                    "short_label": implementation.short_label,
                    "label": implementation.label,
                }
                for implementation in enabled_implementations(
                    precision,
                    qdpp=configuration["harness"]["qdpp"] == "on",
                    tlfloat=configuration["harness"]["tlfloat"] == "on",
                )
                if args.consumer_mode != "fastmath" or implementation.id == "fltx"
            ]
            for precision in precisions
        },
        "sample_mode": args.sample_mode,
        "consumer_mode": args.consumer_mode,
        "phases_requested": sorted(runner_names),
        **profile,
        **provenance,
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "status": "running",
        "phases": {},
        "configuration": configuration,
    }
    metadata_stem = run_metadata_stem(compiler, args.consumer_mode)
    metadata_partial = staging / f"{metadata_stem}.partial.json"
    _write_json(metadata_partial, metadata)
    staged_outputs: list[tuple[Path, Path]] = []
    phase_errors: list[str] = []
    phases = metadata["phases"]
    assert isinstance(phases, dict)

    outputs_by_precision = {
        precision: {
            "accuracy": staging
            / f"{metrics_stem(compiler, precision, args.consumer_mode)}_accuracy.csv",
            "benchmark": staging
            / f"{metrics_stem(compiler, precision, args.consumer_mode)}_benchmark.csv",
        }
        for precision in precisions
    }
    passed_by_precision: dict[str, dict[str, bool]] = {
        precision: {} for precision in precisions
    }
    stop_phases = False

    parallel_accuracy = (
        bool(profile.get("accuracy_parallel", False))
        and "accuracy" in runner_names
        and len(precisions) > 1
    )
    if parallel_accuracy:
        accuracy_executable = next(
            executable for name, executable in runners if name == "accuracy"
        )
        requested_samples = int(profile["accuracy_samples"])
        for precision in precisions:
            phases[f"{precision}.accuracy"] = {"status": "running"}
        _write_json(metadata_partial, metadata)

        def run_accuracy(precision: str):
            output = outputs_by_precision[precision]["accuracy"]
            return _run(
                [
                    *_runner_command(accuracy_executable),
                    "--run-id",
                    run_id,
                    "--source-revision",
                    revision,
                    "--precision",
                    precision,
                    "--sample-mode",
                    args.sample_mode,
                    "--samples",
                    str(requested_samples),
                    *accuracy_policy_arguments(args.consumer_mode),
                    "--output",
                    str(output),
                ],
                "accuracy",
                sample_mode=args.sample_mode,
                samples=requested_samples,
                trials=0,
            )

        with ThreadPoolExecutor(max_workers=len(precisions)) as executor:
            futures: dict[str, Future] = {
                precision: executor.submit(run_accuracy, precision)
                for precision in precisions
            }
            for precision in precisions:
                output = outputs_by_precision[precision]["accuracy"]
                phase_name = f"{precision}.accuracy"
                phase = phases[phase_name]
                assert isinstance(phase, dict)
                try:
                    observed, return_code = futures[precision].result()
                    configuration = _accept_runner_configuration(
                        configuration,
                        observed,
                        source=f"accuracy/{precision}",
                        fingerprint=fingerprint,
                        platform=platform,
                        architecture=architecture,
                        compiler=compiler,
                        publishable=complete_run,
                    )
                    metadata["configuration"] = configuration
                    phase["return_code"] = return_code
                    if return_code:
                        raise MetricsError(
                            "accuracy/"
                            f"{precision}: runner exited with status {return_code}"
                        )
                    phase["status"] = "passed"
                    phase["output"] = output.name
                    passed_by_precision[precision]["accuracy"] = True
                except (MetricsError, OSError) as error:
                    partial = output.with_name(
                        output.stem + ".partial" + output.suffix
                    )
                    phase["status"] = "failed"
                    phase["error"] = str(error)
                    if partial.is_file():
                        phase["output"] = partial.name
                    elif output.is_file():
                        phase["output"] = output.name
                    phase_errors.append(f"{phase_name}: {error}")
                    passed_by_precision[precision]["accuracy"] = False
                    stop_phases = True
                _write_json(metadata_partial, metadata)

    # Gate every precision's accuracy before starting any benchmarks. A failed
    # accuracy phase must not spend several more minutes producing benchmark
    # evidence that cannot be published with that run.
    for runner_name, executable in runners:
        if parallel_accuracy and runner_name == "accuracy":
            continue
        if stop_phases:
            break
        for precision in precisions:
            outputs = outputs_by_precision[precision]
            passed = passed_by_precision[precision]
            output = outputs[runner_name]
            phase_name = f"{precision}.{runner_name}"
            phase: dict[str, object] = {"status": "running"}
            phases[phase_name] = phase
            _write_json(metadata_partial, metadata)
            common = [
                "--run-id",
                run_id,
                "--source-revision",
                revision,
                "--precision",
                precision,
            ]
            try:
                requested_samples = profile[f"{runner_name}_samples"]
                if isinstance(requested_samples, dict):
                    requested_samples = requested_samples[precision]
                requested_trials = (
                    profile["benchmark_trials"] if runner_name == "benchmark" else 0
                )
                observed, return_code = _run(
                    [
                        *_runner_command(executable),
                        *common,
                        "--sample-mode",
                        args.sample_mode,
                        "--samples",
                        str(requested_samples),
                        *(
                            ["--trials", str(requested_trials)]
                            if runner_name == "benchmark"
                            else []
                        ),
                        *(
                            accuracy_policy_arguments(args.consumer_mode)
                            if runner_name == "accuracy"
                            else ()
                        ),
                        "--output",
                        str(output),
                    ],
                    runner_name,
                    sample_mode=args.sample_mode,
                    samples=requested_samples,
                    trials=requested_trials,
                )
                configuration = _accept_runner_configuration(
                    configuration,
                    observed,
                    source=f"{runner_name}/{precision}",
                    fingerprint=fingerprint,
                    platform=platform,
                    architecture=architecture,
                    compiler=compiler,
                    publishable=complete_run,
                )
                metadata["configuration"] = configuration
                phase["return_code"] = return_code
                if return_code:
                    raise MetricsError(
                        f"{runner_name}/{precision}: runner exited with status {return_code}"
                    )
                phase["status"] = "passed"
                phase["output"] = output.name
                passed[runner_name] = True
            except (MetricsError, OSError) as error:
                partial = output.with_name(
                    output.stem + ".partial" + output.suffix
                )
                phase["status"] = "failed"
                phase["error"] = str(error)
                if partial.is_file():
                    phase["output"] = partial.name
                elif output.is_file():
                    phase["output"] = output.name
                phase_errors.append(f"{phase_name}: {error}")
                passed[runner_name] = False
                stop_phases = True
            _write_json(metadata_partial, metadata)
            if stop_phases:
                break
        if stop_phases:
            break

    for precision in precisions:
        outputs = outputs_by_precision[precision]
        accuracy = outputs["accuracy"]
        benchmark = outputs["benchmark"]
        passed = passed_by_precision[precision]
        all_requested_passed = (
            set(passed) == runner_names and all(passed.values())
        )
        if (
            runner_names == {"accuracy", "benchmark"}
            and all_requested_passed
        ):
            canonical = staging / (
                f"{metrics_stem(compiler, precision, args.consumer_mode)}.csv"
            )
            try:
                merge_precision(
                    accuracy,
                    benchmark,
                    canonical,
                    run_id=run_id,
                    revision=revision,
                    fingerprint=fingerprint,
                    precision=precision,
                    qdpp_enabled=configuration["harness"]["qdpp"] == "on",
                    tlfloat_enabled=(
                        configuration["harness"]["tlfloat"] == "on"
                    ),
                    consumer_mode=args.consumer_mode,
                    require_complete=complete_run,
                    expected_trials=_expected_benchmark_trials(
                        args.sample_mode, profile,
                    ),
                )
                staged_outputs.extend(
                    (
                        (accuracy, detail / accuracy.name),
                        (benchmark, detail / benchmark.name),
                        (canonical, target / canonical.name),
                    )
                )
            except (MetricsError, OSError) as error:
                phase_errors.append(f"{precision}.merge: {error}")
        elif all_requested_passed:
            staged_outputs.extend(
                (outputs[name], detail / outputs[name].name)
                for name, _ in runners
            )

    if phase_errors:
        metadata["status"] = "failed"
        metadata["errors"] = phase_errors
        _write_json(metadata_partial, metadata)
        print("metrics failed:", file=sys.stderr)
        for error in phase_errors:
            print(f"  {error}", file=sys.stderr)
        print(f"partial evidence remains in {staging}", file=sys.stderr)
        return 1
    try:
        metadata["outputs"] = _output_hashes(staged_outputs, target)
    except MetricsError as error:
        metadata["status"] = "failed"
        metadata["error"] = f"cannot hash published outputs: {error}"
        _write_json(metadata_partial, metadata)
        print(f"metrics failed: {metadata['error']}", file=sys.stderr)
        return 1
    metadata["status"] = "complete"
    final_metadata = detail / f"{metadata_stem}.json"
    _write_json(metadata_partial, metadata)
    try:
        with _publication_lock(detail / f".{metadata_stem}.publish.lock"):
            _publish_transaction(
                staged_outputs,
                metadata_partial,
                final_metadata,
            )
    except (MetricsError, OSError) as error:
        metadata["status"] = "failed"
        metadata["error"] = f"publish failed: {error}"
        _write_json(metadata_partial, metadata)
        print(f"metrics failed: {metadata['error']}", file=sys.stderr)
        return 1
    try:
        staging.rmdir()
        staging.parent.rmdir()
    except OSError:
        pass
    action = "published" if args.canonical_publication else "completed"
    if args.result_file is not None:
        try:
            _write_json(
                args.result_file.resolve(),
                _result_handoff(
                    run_id=run_id,
                    sample_mode=args.sample_mode,
                    consumer_mode=args.consumer_mode,
                    platform=platform,
                    architecture=architecture,
                    compiler=compiler,
                    input_root=target.parents[1],
                    metadata=final_metadata,
                ),
            )
        except OSError as error:
            print(
                f"metrics failed: cannot write result file {args.result_file}: {error}",
                file=sys.stderr,
            )
            return 1
    print(f"{action} run {run_id} to {target}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
