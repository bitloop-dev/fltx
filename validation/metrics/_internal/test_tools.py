#!/usr/bin/env python3
"""Focused tests for the normalized metrics tools."""

from __future__ import annotations

import copy
import colorsys
import csv
import json
import io
import math
import re
import subprocess
import tempfile
import threading
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest import mock

import build_tables
import build_performance
import build_overview
import build_profile_comparison
import manifest
import preset_pipeline
import report_pipeline
import run_metrics
import source_fingerprint
from svg_table import Cell, Column, Row, Table, WHITE, render


FINGERPRINT = "a" * 64


def accuracy_row(
    implementation: str = "fltx",
    *,
    group: str = "arithmetic",
    operation: str = "add",
    domain: str = "moderate",
) -> dict[str, str]:
    identity = {
        "fltx": ("fltx", "fltx bl::f128"),
        "qdpp": ("ddreal", "qdpp dd_real"),
        "cppdd": ("cppdd", "boost::multiprecision::cpp_double_double"),
        "tlfloat": ("tlquad", "TLFloat Quad"),
    }[implementation]
    gated = implementation == "fltx"
    return {
        "schema_version": run_metrics.SCHEMA_VERSION,
        "run_id": "run",
        "source_revision": "rev",
        "source_fingerprint": FINGERPRINT,
        "precision": "f128",
        "group": group,
        "operation": operation,
        "implementation": implementation,
        "implementation_short": identity[0],
        "implementation_label": identity[1],
        "api": f"<{implementation} & api>",
        "domain": domain,
        "samples": "4",
        "seed": "7",
        "mean_bits": "110",
        "p01_bits": "105",
        "worst_bits": "100",
        "required_worst_bits": "90" if gated else "",
        "margin_bits": "10" if gated else "",
        "pass": "yes" if gated else "",
        "special_support": "Both",
        "worst_input": "1",
        "observed": "1",
        "reference": "1",
    }


def benchmark_row(
    implementation: str = "fltx",
    *,
    ns: float = 10.0,
    ratio: float | None = None,
    group: str = "arithmetic",
    operation: str = "add",
) -> dict[str, str]:
    identity = {
        "fltx": ("fltx", "fltx bl::f128"),
        "qdpp": ("ddreal", "qdpp dd_real"),
        "cppdd": ("cppdd", "boost::multiprecision::cpp_double_double"),
        "tlfloat": ("tlquad", "TLFloat Quad"),
    }[implementation]
    return {
        "schema_version": run_metrics.SCHEMA_VERSION,
        "run_id": "run",
        "source_revision": "rev",
        "source_fingerprint": FINGERPRINT,
        "precision": "f128",
        "group": group,
        "operation": operation,
        "implementation": implementation,
        "implementation_short": identity[0],
        "implementation_label": identity[1],
        "api": f"<{implementation} & api>",
        "samples": "16",
        "ns_iter": str(ns),
        "trials_ns": f"{ns}|{ns}|{ns}",
        "iterations": "48",
        "elapsed_ns": str(ns * 48),
        "speed_ratio": "" if ratio is None else str(ratio),
    }


def synthetic_dataset(order: tuple[str, ...]) -> tuple[build_tables.Dataset, build_tables.Target]:
    target = build_tables.Target("windows", "MSVC")
    rows = {
        "fltx": {
            **benchmark_row(),
            **{
                "mean_bits": "110", "p01_bits": "105", "worst_bits": "100",
                "domains_passed": "1", "domains_total": "1",
                "min_margin_bits": "10", "accuracy_pass": "yes",
                "special_support": "Both",
                "run_id": "run",
            },
        },
        "qdpp": {
            **benchmark_row("qdpp", ns=8, ratio=0.8),
            **{
                "mean_bits": "100", "p01_bits": "95", "worst_bits": "90",
                "domains_passed": "1", "domains_total": "1",
                "min_margin_bits": "0", "accuracy_pass": "yes",
                "special_support": "Both", "run_id": "run",
            },
        },
        "cppdd": {
            **benchmark_row("cppdd", ns=20, ratio=2),
            **{
                "mean_bits": "108", "p01_bits": "102", "worst_bits": "98",
                "domains_passed": "1", "domains_total": "1",
                "min_margin_bits": "8", "accuracy_pass": "yes",
                "special_support": "Both", "run_id": "run",
            },
        },
        "tlfloat": {
            **benchmark_row("tlfloat", ns=12, ratio=1.2),
            **{
                "mean_bits": "112", "p01_bits": "108", "worst_bits": "101",
                "domains_passed": "1", "domains_total": "1",
                "min_margin_bits": "11", "accuracy_pass": "yes",
                "special_support": "Both", "run_id": "run",
            },
        },
    }
    canonical = {
        (target, "f128", "arithmetic", "add", implementation):
            rows[implementation]
        for implementation in order
    }
    return (
        build_tables.Dataset(
            canonical=canonical,
            accuracy={},
            implementations={(target, "f128"): order, (target, "f256"): ("fltx",)},
            revisions={"rev"},
            fingerprints={FINGERPRINT},
            runs={target: "run"},
        ),
        target,
    )


def synthetic_performance_dataset(
    order: tuple[str, ...],
) -> tuple[build_performance.PerformanceDataset, build_tables.Target]:
    source, target = synthetic_dataset(order)
    column = build_performance.ColumnKey("f128", target)
    rows = {
        (column, group, operation, implementation): row
        for (
            row_target,
            precision,
            group,
            operation,
            implementation,
        ), row in source.canonical.items()
        if row_target == target and precision == "f128"
    }
    return (
        build_performance.PerformanceDataset(
            columns=(column,),
            rows=rows,
            implementations={
                (column, "arithmetic", "add"): order,
            },
            run_ids={column: "run"},
            sources={column: Path("windows/MSVC_f128.csv")},
        ),
        target,
    )


def write_canonical(path: Path, rows: list[dict[str, str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=run_metrics.CANONICAL_FIELDS)
        writer.writeheader()
        writer.writerows({
            field: row.get(field, "")
            for field in run_metrics.CANONICAL_FIELDS
        } for row in rows)


class SourceFingerprintTests(unittest.TestCase):
    def test_report_tool_changes_do_not_invalidate_runner_sources(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "include").mkdir()
            (root / "validation" / "metrics").mkdir(parents=True)
            header = root / "include" / "value.hpp"
            renderer = root / "validation" / "metrics" / "_internal" / "build_tables.py"
            renderer.parent.mkdir()
            header.write_text("#pragma once\n", encoding="utf-8")
            renderer.write_text("COLOUR = 'before'\n", encoding="utf-8")
            subprocess.run(["git", "init", "-q"], cwd=root, check=True)
            subprocess.run(
                ["git", "config", "user.email", "metrics@example.invalid"],
                cwd=root,
                check=True,
            )
            subprocess.run(
                ["git", "config", "user.name", "Metrics Test"],
                cwd=root,
                check=True,
            )
            subprocess.run(["git", "add", "."], cwd=root, check=True)
            subprocess.run(
                ["git", "commit", "-qm", "initial"],
                cwd=root,
                check=True,
            )

            initial = source_fingerprint.source_identity(root)
            renderer.write_text("COLOUR = 'new'\n", encoding="utf-8")
            report_only = source_fingerprint.source_identity(root)
            self.assertEqual(report_only.fingerprint, initial.fingerprint)
            self.assertEqual(report_only.revision, initial.revision)
            subprocess.run(["git", "add", "."], cwd=root, check=True)
            subprocess.run(
                ["git", "commit", "-qm", "renderer"],
                cwd=root,
                check=True,
            )
            committed_report = source_fingerprint.source_identity(root)
            self.assertEqual(committed_report.fingerprint, initial.fingerprint)
            self.assertNotEqual(committed_report.revision, initial.revision)

            header.write_text("#pragma once\n#define VALUE 1\n", encoding="utf-8")
            runner_changed = source_fingerprint.source_identity(root)
            self.assertNotEqual(runner_changed.fingerprint, initial.fingerprint)
            self.assertIn("-dirty-", runner_changed.revision)

    def test_runner_scope_includes_build_inputs_but_not_local_presets(self) -> None:
        self.assertTrue(source_fingerprint._included("include/fltx/fltx.hpp"))
        self.assertTrue(source_fingerprint._included("validation/accuracy/main.cpp"))
        self.assertTrue(source_fingerprint._included("validation/CMakeLists.txt"))
        self.assertTrue(source_fingerprint._included("vcpkg.json"))
        self.assertFalse(source_fingerprint._included("validation/metrics/_internal/build_tables.py"))
        self.assertFalse(source_fingerprint._included("validation/metrics/_internal/build_performance.py"))
        self.assertFalse(source_fingerprint._included("CMakeUserPresets.json"))


class ManifestTests(unittest.TestCase):
    def test_operation_families_match_the_public_metrics_taxonomy(self) -> None:
        operations = manifest.EXPECTED_BENCHMARK["f128"]
        arithmetic = {
            operation
            for group, operation in operations
            if group == "arithmetic"
        }
        self.assertEqual(
            arithmetic,
            {"add", "subtract", "multiply", "divide"},
        )
        self.assertEqual(
            tuple(build_tables.OPERATION_ORDER["arithmetic"]),
            ("add", "subtract", "multiply", "divide"),
        )
        self.assertEqual(
            tuple(build_tables.GROUP_ORDER),
            (
                "arithmetic", "io", "rounding", "remainders",
                "floating_point_utilities", "roots_and_powers",
                "exponentials", "logarithms", "trigonometric", "hyperbolic",
                "inverse_hyperbolic", "special_functions", "comparisons",
                "random", "mixed_workloads",
            ),
        )
        self.assertIn(("roots_and_powers", "sqr"), operations)
        self.assertNotIn(("floating_point_utilities", "sqr"), operations)
        roots = tuple(build_tables.OPERATION_ORDER["roots_and_powers"])
        self.assertLess(roots.index("sqr"), roots.index("sqrt"))
        self.assertFalse({
            "remainder", "decomposition", "stepping", "comparison",
            "roots", "powers", "trig", "special", "expressions", "workloads",
        } & {group for group, _ in operations})

    def test_current_counts_are_derived_per_implementation(self) -> None:
        self.assertEqual(
            {key: len(value) for key, value in
             manifest.benchmark_manifest(
                 "f128", qdpp=True, tlfloat=True,
             ).items()},
            {"fltx": 85, "qdpp": 60, "cppdd": 75, "tlfloat": 71},
        )
        self.assertEqual(
            {key: len(value) for key, value in
             manifest.accuracy_manifest(
                 "f256", qdpp=True, tlfloat=True,
             ).items()},
            {"fltx": 123, "qdpp": 91, "mpfr64": 114, "tlfloat": 107},
        )

    def test_extended_accuracy_uses_one_weighted_arithmetic_domain(self) -> None:
        rows = manifest.accuracy_manifest(
            "f256", qdpp=True, tlfloat=True,
        )["fltx"]
        arithmetic = {
            row for row in rows
            if row[0] == "arithmetic"
        }
        self.assertEqual(
            arithmetic,
            {
                ("arithmetic", "add", "general"),
                ("arithmetic", "subtract", "general"),
                ("arithmetic", "multiply", "general"),
                ("arithmetic", "divide", "general"),
            },
        )

    def test_fmod_quotient_reduction_domain_is_extended_precision_only(self) -> None:
        quotient_reduction = ("remainders", "fmod", "quotient_reduction")
        for precision in ("f128", "f256"):
            self.assertIn(
                quotient_reduction,
                manifest.accuracy_manifest(
                    precision, qdpp=True, tlfloat=True,
                )["fltx"],
            )
        for precision in ("f32", "f64"):
            self.assertNotIn(
                quotient_reduction,
                manifest.EXPECTED_ACCURACY[precision],
            )

    def test_native_accuracy_uses_precision_specific_domains(self) -> None:
        rows = manifest.EXPECTED_ACCURACY["f64"]
        self.assertIn(("arithmetic", "add", "near_one"), rows)
        self.assertNotIn(("arithmetic", "add", "general"), rows)

    def test_trig_accuracy_requires_quadrant_boundaries(self) -> None:
        for precision in ("f128", "f256"):
            rows = manifest.accuracy_manifest(
                precision,
                qdpp=True,
                tlfloat=True,
            )["fltx"]
            for operation in ("sin", "cos", "tan"):
                self.assertIn(
                    (
                        "trigonometric",
                        operation,
                        "quadrant_boundaries",
                    ),
                    rows,
                )

    def test_qdpp_disabled_keeps_boost(self) -> None:
        self.assertEqual(
            tuple(manifest.benchmark_manifest(
                "f128", qdpp=False, tlfloat=True,
            )),
            ("fltx", "cppdd", "tlfloat"),
        )

    def test_formatter_accuracy_rows_follow_supported_apis(self) -> None:
        accuracy = manifest.accuracy_manifest(
            "f128", qdpp=True, tlfloat=True,
        )
        formatter_rows = {
            ("io", operation, domain)
            for operation in ("to_string", "to_chars")
            for domain in ("moderate", "wide_exponent")
        }
        self.assertTrue(formatter_rows <= accuracy["fltx"])
        self.assertTrue(formatter_rows <= accuracy["qdpp"])
        self.assertIn(("io", "to_string", "moderate"), accuracy["cppdd"])
        self.assertNotIn(("io", "to_chars", "moderate"), accuracy["cppdd"])
        self.assertIn(("io", "to_string", "moderate"), accuracy["tlfloat"])
        self.assertNotIn(("io", "to_chars", "moderate"), accuracy["tlfloat"])

    def test_comparison_accuracy_covers_every_enabled_implementation(self) -> None:
        accuracy = manifest.accuracy_manifest(
            "f256", qdpp=True, tlfloat=True,
        )
        rows = {
            ("comparisons", operation, "moderate")
            for operation in (
                "equal", "not_equal", "less", "less_equal",
                "greater", "greater_equal",
            )
        }
        for implementation_rows in accuracy.values():
            self.assertTrue(rows <= implementation_rows)

    def test_noteworthy_public_api_labels(self) -> None:
        self.assertEqual(
            manifest.api_name("qdpp", "f128", "io", "parse"),
            "qdpp read",
        )
        self.assertEqual(
            manifest.api_name("cppdd", "f128", "roots_and_powers", "ipow"),
            "boost::multiprecision::pow(value, int)",
        )
        self.assertEqual(
            manifest.api_name("tlfloat", "f128", "rounding", "roundeven"),
            "tlfloat::rint",
        )


class ValidationTests(unittest.TestCase):
    @staticmethod
    def configuration_banner(*, simulated_consteval: str = "off"):
        consumer = {
            "fma": "AUTO",
            "simd": "off",
            "fast-math": "off",
            "simulated-consteval": simulated_consteval,
            "x86": "1",
            "arm64": "0",
            "tu-x86-fma": "0",
            "has-x86-fma": "0",
        }
        return {
            "harness": {"qdpp": "off", "tlfloat": "off"},
            "build": {
                "compiler-id": "MSVC",
                "compiler-version": "test",
                "config": "Release",
                "system": "Windows",
                "processor": "x64",
                "optimized": "1",
                "flags-hash": FINGERPRINT,
                "source-fingerprint": FINGERPRINT,
            },
            "consumer": consumer,
            "library": {
                "fma": "AUTO",
                "simd": "off",
                "fast-math-request": "AUTO",
                "fast-math": "off",
                "x86": "1",
                "arm64": "0",
                "tu-x86-fma": "0",
                "runtime-fma": "unavailable",
            },
            "library_strategy": {
                field: "0"
                for field in run_metrics.REQUIRED_CONFIGURATION_FIELDS[
                    "library_strategy"
                ]
            },
        }

    def test_configuration_requires_the_execution_mode(self) -> None:
        configuration = run_metrics._validate_configuration(
            self.configuration_banner(simulated_consteval="on"),
            "configuration banner",
        )
        self.assertEqual(configuration["consumer"]["simulated-consteval"], "on")

        incomplete = self.configuration_banner()
        del incomplete["consumer"]["simulated-consteval"]
        with self.assertRaisesRegex(
            run_metrics.MetricsError,
            "missing simulated-consteval",
        ):
            run_metrics._validate_configuration(incomplete, "configuration banner")

    def test_consumer_modes_are_verified_and_named_independently(self) -> None:
        strict = self.configuration_banner()
        self.assertEqual(
            run_metrics.require_consumer_mode(
                strict,
                "strict",
                "strict runner",
            )["consumer"]["fast-math"],
            "off",
        )

        fastmath = self.configuration_banner()
        fastmath["consumer"]["fast-math"] = "on"
        self.assertEqual(
            run_metrics.require_consumer_mode(
                fastmath,
                "fastmath",
                "fast-math runner",
            )["consumer"]["fast-math"],
            "on",
        )
        with self.assertRaisesRegex(run_metrics.MetricsError, "expected off"):
            run_metrics.require_consumer_mode(
                fastmath,
                "strict",
                "mislabelled runner",
            )

        self.assertEqual(
            run_metrics.metrics_stem("MSVC", "f128", "strict"),
            "MSVC_f128",
        )
        self.assertEqual(
            run_metrics.metrics_stem("MSVC", "f128", "fastmath"),
            "MSVC_f128_fastmath",
        )
        self.assertEqual(
            run_metrics.run_metadata_stem("MSVC", "fastmath"),
            "MSVC_fastmath_run",
        )

    def test_metrics_runner_accepts_either_phase_independently(self) -> None:
        common = ["--platform", "windows", "--compiler", "MSVC"]
        accuracy = run_metrics.parse_args([
            "--accuracy", "accuracy.exe", *common,
        ])
        benchmark = run_metrics.parse_args([
            "--benchmark", "benchmark.exe", *common,
        ])
        self.assertEqual(accuracy.accuracy, Path("accuracy.exe"))
        self.assertIsNone(accuracy.benchmark)
        self.assertIsNone(benchmark.accuracy)
        self.assertEqual(benchmark.benchmark, Path("benchmark.exe"))

    def test_metrics_runner_can_infer_target(self) -> None:
        args = run_metrics.parse_args([
            "--accuracy", "accuracy.exe",
            "--benchmark", "benchmark.exe",
        ])
        self.assertIsNone(args.platform)
        self.assertIsNone(args.compiler)

    def test_preset_pipeline_can_explicitly_force_a_rerun(self) -> None:
        args = preset_pipeline.parse_args([
            "--preset", "native-release",
            "--force-rerun",
        ])
        self.assertTrue(args.force_rerun)
        self.assertFalse(args.release)

    def test_preset_pipeline_defaults_to_local_standard_profile(self) -> None:
        args = preset_pipeline.parse_args([
            "--preset", "native-release",
        ])
        self.assertFalse(args.release)
        self.assertEqual(
            run_metrics.SAMPLE_PROFILES["standard"],
            {
                "accuracy_samples": 4096,
                "benchmark_samples": {"f128": 8192, "f256": 4096},
                "benchmark_trials": 7,
                "benchmark_minimum_trial_ms": 8,
                "benchmark_policy": "adaptive-v1",
                "benchmark_fast_threshold_ns": 20,
                "benchmark_slow_threshold_ns": 10_000,
                "benchmark_fast_trials": 7,
                "benchmark_ordinary_trials": 3,
                "benchmark_fast_minimum_trial_ms": 15,
                "benchmark_maximum_row_ms": 5_000,
                "benchmark_corpus_policy": "adaptive-v1",
                "accuracy_parallel": True,
            },
        )

    def test_preset_pipeline_release_flag_selects_publication_profile(self) -> None:
        args = preset_pipeline.parse_args([
            "--preset", "native-release",
            "--release",
        ])
        self.assertTrue(args.release)
        self.assertEqual(
            run_metrics.SAMPLE_PROFILES["full"],
            {
                "accuracy_samples": 65536,
                "benchmark_samples": {"f128": 81920, "f256": 40960},
                "benchmark_trials": 7,
                "benchmark_minimum_trial_ms": 25,
            },
        )

    def test_preset_pipeline_rejects_removed_report_options(self) -> None:
        for removed in (
            ("--mode", "quick"),
            ("--sample-mode", "smoke"),
            ("--layout", "compact"),
        ):
            with self.subTest(option=removed[0]), redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    preset_pipeline.parse_args([
                        "--preset", "native-release", *removed,
                    ])

    def test_compatible_evidence_handoff_reuses_the_existing_run(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            input_root = Path(temporary)
            metadata_path = input_root / "windows" / "detail" / "MSVC_run.json"
            metadata_path.parent.mkdir(parents=True)
            configuration = {"build": {"source-fingerprint": FINGERPRINT}}
            profile = run_metrics.SAMPLE_PROFILES["full"]
            metadata = {
                "schema_version": run_metrics.SCHEMA_VERSION,
                "run_id": "cached",
                "source_revision": "revision",
                "source_fingerprint": FINGERPRINT,
                "platform": "windows",
                "compiler": "MSVC",
                "precisions": list(run_metrics.PRECISIONS),
                "sample_mode": "full",
                "phases_requested": ["accuracy", "benchmark"],
                "configuration": configuration,
                "host": {"machine": "test"},
                "status": "complete",
                "outputs": {},
                "phases": {
                    f"{precision}.{runner}": {"status": "passed"}
                    for precision in run_metrics.PRECISIONS
                    for runner in ("accuracy", "benchmark")
                },
                **profile,
            }
            metadata_path.write_text(json.dumps(metadata), encoding="utf-8")
            summary = {field: "" for field in run_metrics.CANONICAL_FIELDS}
            summary.update({
                "group": "arithmetic",
                "operation": "add",
                "implementation": "fltx",
                "run_id": "cached",
            })

            def rows_for(
                path: Path,
                fields: tuple[str, ...],
                **_: object,
            ) -> list[dict[str, str]]:
                if fields == run_metrics.CANONICAL_FIELDS:
                    return [dict(summary)]
                return [{"placeholder": "row"}]

            with (
                mock.patch.object(run_metrics, "_verify_output_hashes"),
                mock.patch.object(
                    run_metrics,
                    "_validate_configuration",
                    return_value={
                        "harness": {"qdpp": "off", "tlfloat": "off"},
                        "consumer": {"fast-math": "off"},
                    },
                ),
                mock.patch.object(run_metrics, "_read_csv", side_effect=rows_for),
                mock.patch.object(run_metrics, "_validate_identity"),
                mock.patch.object(run_metrics, "_validate_completeness"),
                mock.patch.object(run_metrics, "_validate_implementations"),
                mock.patch.object(run_metrics, "aggregate", return_value=[summary]),
            ):
                result = run_metrics._validate_reusable_candidate(
                    metadata_path,
                    sample_mode="full",
                    platform="windows",
                    compiler="MSVC",
                    fingerprint=FINGERPRINT,
                    configuration=configuration,
                    host={"machine": "test"},
                )
            self.assertEqual(result["run_id"], "cached")
            self.assertEqual(result["input_root"], str(input_root.resolve()))

    def test_metrics_runner_requires_explicit_target_as_a_pair(self) -> None:
        with redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                run_metrics.parse_args([
                    "--accuracy", "accuracy.exe",
                    "--platform", "windows",
                ])

    def test_canonical_target_inference_covers_supported_builds(self) -> None:
        identities = {
            ("Windows", "MSVC"): ("windows", "MSVC"),
            ("Windows", "GNU"): ("windows", "MinGW"),
            ("Emscripten", "Clang"): ("wasm32", "Wasm32"),
            ("Linux", "GNU"): ("linux", "GCC"),
            ("Linux", "Clang"): ("linux", "Clang"),
            ("Darwin", "AppleClang"): ("macos", "AppleClang"),
        }
        for (system, compiler_id), expected in identities.items():
            configuration = {
                "build": {
                    "system": system,
                    "compiler-id": compiler_id,
                    "processor": "test",
                },
            }
            self.assertEqual(
                run_metrics.infer_canonical_target(configuration, "test"),
                expected,
            )

    def test_canonical_target_inference_rejects_unknown_build(self) -> None:
        with self.assertRaisesRegex(
            run_metrics.MetricsError,
            "unsupported canonical build identity",
        ):
            run_metrics.infer_canonical_target(
                {
                    "build": {
                        "system": "Windows",
                        "compiler-id": "Clang",
                        "processor": "x86_64",
                    },
                },
                "test",
            )

    def test_duplicate_implementation_operation_is_rejected(self) -> None:
        rows = [benchmark_row(), benchmark_row()]
        with self.assertRaisesRegex(run_metrics.MetricsError, "duplicate"):
            run_metrics._validate_identity(
                rows, Path("benchmark.csv"), run_id="run", revision="rev",
                fingerprint=FINGERPRINT, precision="f128",
            )

    def test_missing_enabled_implementation_is_rejected(self) -> None:
        accuracy = [
            {
                "group": group, "operation": operation, "domain": domain,
                "implementation": implementation,
            }
            for implementation, expected in
            manifest.accuracy_manifest(
                "f128", qdpp=False, tlfloat=False,
            ).items()
            for group, operation, domain in expected
        ]
        benchmark = [
            {
                "group": group, "operation": operation,
                "implementation": implementation,
            }
            for implementation, expected in
            manifest.benchmark_manifest(
                "f128", qdpp=False, tlfloat=False,
            ).items()
            for group, operation in expected
            if not (implementation == "cppdd" and operation == "add")
        ]
        with self.assertRaisesRegex(run_metrics.MetricsError, "incomplete cppdd"):
            run_metrics._validate_completeness(
                accuracy, benchmark, precision="f128",
                accuracy_path=Path("accuracy.csv"),
                benchmark_path=Path("benchmark.csv"),
                qdpp_enabled=False,
                tlfloat_enabled=False,
            )

    def test_normalized_aggregation_keeps_one_row_per_implementation(self) -> None:
        result = run_metrics.aggregate(
            [accuracy_row(), accuracy_row("cppdd")],
            [benchmark_row(), benchmark_row("cppdd", ns=20, ratio=2)],
            accuracy_path=Path("accuracy.csv"),
            benchmark_path=Path("benchmark.csv"),
            run_id="run",
            expected_trials=3,
        )
        self.assertEqual([row["implementation"] for row in result], ["cppdd", "fltx"])
        cppdd = result[0]
        self.assertEqual(cppdd["speed_ratio"], 2)
        self.assertEqual(cppdd["accuracy_pass"], "yes")
        self.assertEqual(cppdd["domains_passed"], 1)

    def test_standard_accepts_only_policy_compliant_trial_counts(self) -> None:
        for count in (1, 3, 7):
            row = benchmark_row()
            row["trials_ns"] = "|".join([row["ns_iter"]] * count)
            run_metrics._benchmark_values(
                row,
                Path("benchmark.csv"),
                expected_trials=(1, 3, 7),
            )

        row = benchmark_row()
        row["trials_ns"] = "10|10|10|10|10"
        with self.assertRaisesRegex(run_metrics.MetricsError, "invalid benchmark trials"):
            run_metrics._benchmark_values(
                row,
                Path("benchmark.csv"),
                expected_trials=(1, 3, 7),
            )

    def test_metrics_reject_implausibly_small_benchmark_timings(self) -> None:
        row = benchmark_row(ns=0.0001)
        with self.assertRaisesRegex(
            run_metrics.MetricsError,
            "workload may have been optimized away",
        ):
            run_metrics._benchmark_values(row, Path("benchmark.csv"))

        with self.assertRaisesRegex(
            run_metrics.MetricsError,
            "workload may have been optimized away",
        ):
            build_performance._positive_float(
                "0.0001",
                Path("canonical.csv"),
                ("mixed_workloads", "mandelbrot", "qdpp"),
            )

    def test_trial_rotation_is_balanced_for_any_count(self) -> None:
        for count in range(1, 6):
            for trials in range(1, 12):
                first_positions = [
                    trial % count for trial in range(trials)
                ]
                counts = [first_positions.count(index) for index in range(count)]
                self.assertLessEqual(max(counts) - min(counts), 1)


class RenderingTests(unittest.TestCase):
    def test_all_headline_accuracy_is_uncapped_bits_accurate(self) -> None:
        dataset, target = synthetic_dataset(("fltx", "cppdd", "qdpp"))
        fltx = dataset.canonical[
            (target, "f128", "arithmetic", "add", "fltx")
        ]
        fltx["mean_bits"] = "118.5"
        fltx["worst_bits"] = "114"

        cell = build_tables.accuracy_table(dataset, (target,)).rows[0].cells[0]
        self.assertEqual(cell.lines[:2], ("avg 118.5b", "min 114.0b"))

        overview = build_overview.render_overview(dataset, target, "f128")
        self.assertIn(">118.5</text>", overview)
        self.assertIn(">114.0</text>", overview)
        self.assertIn(">bits accurate</text>", overview)

        tooltip = build_overview._tooltip(
            dataset,
            target,
            "f128",
            "arithmetic",
            "add",
            "fltx",
            dataset.canonical[
                (target, "f128", "arithmetic", "add", "fltx")
            ],
        )
        self.assertNotIn("nominal", tooltip)
        self.assertNotIn("capped", tooltip)
        self.assertIn("mean: 118.5 bits", tooltip)

    def test_multiline_overview_headers_have_real_line_spacing(self) -> None:
        parts: list[str] = []
        build_overview._text_lines(
            parts,
            20,
            10,
            60,
            ("Inf/", "NaN"),
        )
        baselines = [
            float(re.search(r' y="([^"]+)"', line).group(1))
            for line in parts
        ]
        self.assertEqual(len(baselines), 2)
        self.assertEqual(baselines[1] - baselines[0], 14)

    def test_all_svg_fonts_are_scaled_without_changing_layout_constants(self) -> None:
        table_svg = render(Table(
            "x", "y", (Column("c"),), (Row("g", "op", (Cell(("a",)),)),),
        ))
        self.assertIn('font-size="28.8"', table_svg)
        self.assertIn('font-size="13.2"', table_svg)
        self.assertIn('stroke="#000000"', table_svg)

        dataset, target = synthetic_dataset(("fltx", "cppdd"))
        overview_svg = build_overview.render_overview(dataset, target, "f128")
        self.assertIn('font-size="18"', overview_svg)
        self.assertIn('font-size="10.8"', overview_svg)
        self.assertEqual(build_overview.CELL_PADDING, 8)
        self.assertEqual(build_overview.ROW_HEIGHT, 22)

    def test_float_utilities_use_the_compact_group_label(self) -> None:
        self.assertEqual(
            build_tables.GROUP_LABELS["floating_point_utilities"],
            "Float Utils",
        )
        self.assertEqual(
            build_tables.GROUP_LABELS["inverse_hyperbolic"],
            "Hyperbolic",
        )
        self.assertEqual(
            build_tables.GROUP_LABELS["special_functions"],
            "Special",
        )
        self.assertEqual(
            build_overview._grouped_operations([
                ("hyperbolic", "sinh"),
                ("inverse_hyperbolic", "asinh"),
            ]),
            [(
                "hyperbolic",
                [
                    ("hyperbolic", "sinh"),
                    ("inverse_hyperbolic", "asinh"),
                ],
            )],
        )

    def test_reports_hide_the_raw_random_engine_benchmark(self) -> None:
        dataset, target = synthetic_dataset(("fltx",))
        source = dataset.canonical[
            (target, "f128", "arithmetic", "add", "fltx")
        ]
        row = dict(source)
        row["group"] = "random"
        row["operation"] = "mt19937_64"
        dataset.canonical[
            (target, "f128", "random", "mt19937_64", "fltx")
        ] = row
        self.assertNotIn(
            ("random", "mt19937_64"),
            build_tables._operations(dataset),
        )
        self.assertNotIn(
            ">mt19937_64<",
            build_overview.render_overview(
                dataset,
                target,
                "f128",
                "compact",
            ),
        )
        performance, _ = synthetic_performance_dataset(("fltx",))
        column = performance.columns[0]
        performance_row = dict(performance.rows[
            (column, "arithmetic", "add", "fltx")
        ])
        performance_row["group"] = "random"
        performance_row["operation"] = "mt19937_64"
        performance.rows[
            (column, "random", "mt19937_64", "fltx")
        ] = performance_row
        performance.implementations[
            (column, "random", "mt19937_64")
        ] = ("fltx",)
        self.assertNotIn(
            ("random", "mt19937_64"),
            build_performance.operations(performance),
        )
        self.assertNotIn(
            ">mt19937_64<",
            build_performance.render(performance),
        )

    def test_overview_uses_metrics_directories_by_default(self) -> None:
        args = build_overview.parse_args(["--target", "windows/MSVC"])
        self.assertEqual(args.input, Path(build_overview.__file__).parents[1] / "data")
        self.assertEqual(args.output, Path(build_overview.__file__).parents[1] / "generated")
        self.assertEqual(args.layout, "full")

    def test_performance_uses_metrics_directories_by_default(self) -> None:
        args = build_performance.parse_args([])
        self.assertEqual(
            args.input,
            Path(build_performance.__file__).parents[1] / "data",
        )
        self.assertEqual(
            args.output,
            Path(build_performance.__file__).parents[1] / "generated",
        )
        self.assertEqual(args.layout, "full")

    def test_performance_table_shows_only_the_fastest_competitor(self) -> None:
        dataset, _ = synthetic_performance_dataset(
            ("fltx", "cppdd", "qdpp", "tlfloat"),
        )
        column = dataset.columns[0]
        dataset.rows[
            (column, "arithmetic", "add", "qdpp")
        ]["speed_ratio"] = "99"
        dataset.rows[
            (column, "arithmetic", "add", "qdpp")
        ]["worst_bits"] = ""
        svg = build_performance.render(dataset)
        self.assertIn(">0.80× vs ddreal<", svg)
        self.assertNotIn("× vs cppdd<", svg)
        self.assertNotIn("× vs tlquad<", svg)
        self.assertIn("Fastest competitor: qdpp dd_real", svg)
        self.assertIn("FLTX speed vs fastest competitor: 0.8×", svg)

    def test_fastest_comparison_colours_ratio_text_on_dark_rows(self) -> None:
        dataset, _ = synthetic_performance_dataset(
            ("fltx", "cppdd", "qdpp"),
        )
        svg = build_performance.render(dataset)
        self.assertIn(
            f'fill="{build_tables._ratio_color(0.8)}" font-size="13" '
            'font-weight="400"',
            svg,
        )
        self.assertIn(f'fill="{build_performance.BODY_ROW_FILLS[0]}"', svg)
        self.assertNotIn(
            f'height="40" fill="{build_tables._ratio_color(0.8)}"',
            svg,
        )

    def test_performance_headers_name_the_measured_fltx_type(self) -> None:
        dataset, _ = synthetic_performance_dataset(("fltx", "qdpp"))
        svg = build_performance.render(dataset)
        self.assertIn(">bl::f128<", svg)
        self.assertNotIn(">vs bl::f128<", svg)
        self.assertIn("; bl::f128; arithmetic / add", svg)

    def test_compact_performance_uses_overview_dark_theme_and_font(self) -> None:
        dataset, _ = synthetic_performance_dataset(("fltx", "cppdd"))
        svg = build_performance.render(dataset, "compact")
        self.assertIn("font-family:'Ubuntu Mono'", svg)
        self.assertIn(f'fill="{build_performance.PAGE}"', svg)
        self.assertIn(f'stroke="{build_performance.GRID}"', svg)
        self.assertEqual(build_performance.PAGE, "#0f1115")
        self.assertEqual(build_performance.GROUP, build_overview.GROUP_ROW_FILL)
        self.assertEqual(build_performance.BODY_ROW_FILLS, build_overview.OPERATION_ROW_FILLS)
        self.assertEqual(build_performance.CELL_PADDING, build_overview.CELL_PADDING)

    def test_accuracy_table_reports_only_fltx_consistency(self) -> None:
        dataset, target = synthetic_dataset(("fltx", "cppdd"))
        cell = build_tables.accuracy_table(dataset, (target,)).rows[0].cells[0]
        self.assertEqual(cell.fill, WHITE)
        self.assertEqual(
            cell.foreground,
            build_tables._margin_text_color(10.0, True),
        )
        self.assertEqual(
            cell.lines,
            ("avg 110.0b", "min 100.0b", "✓ 1/1 domains"),
        )
        self.assertNotIn("cppdd", "\n".join(cell.lines))
        self.assertIn("mean 110 bits", cell.tooltip)
        self.assertIn("p01 105 bits", cell.tooltip)
        self.assertNotIn("cppdd", cell.tooltip)

    def test_dynamic_row_height_grows_with_implementation_lines(self) -> None:
        one = render(Table(
            "x", "y", (Column("c"),), (Row("g", "op", (Cell(("a",)),)),),
        ))
        many = render(Table(
            "x", "y", (Column("c"),),
            (Row("g", "op", (Cell(("a", "b", "c", "d")),)),),
        ))
        height = lambda svg: int(re.search(r'<svg[^>]+height="(\d+)"', svg).group(1))
        self.assertGreater(height(many), height(one))

    def test_xml_escapes_arbitrary_labels_and_api_names(self) -> None:
        dataset, _ = synthetic_performance_dataset(("fltx", "cppdd"))
        svg = build_performance.render(dataset)
        self.assertIn("&lt;cppdd &amp; api&gt;", svg)
        self.assertNotIn("<cppdd & api>", svg)

    def test_nanosecond_format_is_fixed_not_scientific(self) -> None:
        self.assertEqual(
            build_performance._format_ns(8770, "full"),
            "8,770ns",
        )

    def test_performance_discovery_combines_independent_precisions_and_runs(
        self,
    ) -> None:
        source, target = synthetic_dataset(("fltx", "cppdd"))
        add_rows = [
            dict(source.canonical[
                (target, "f128", "arithmetic", "add", implementation)
            ])
            for implementation in ("fltx", "cppdd")
        ]
        f256_abs_rows = []
        for row in add_rows:
            candidate = dict(row)
            candidate["precision"] = "f256"
            candidate["group"] = "arithmetic"
            candidate["operation"] = "abs"
            candidate["run_id"] = "different-run"
            f256_abs_rows.append(candidate)

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            write_canonical(root / "windows" / "MSVC_f128.csv", add_rows)
            write_canonical(
                root / "wasm32" / "Wasm32_f256.csv",
                f256_abs_rows,
            )
            dataset = build_performance.discover(root)
            _, metadata_path = build_performance.build(
                root,
                root / "generated",
            )
            metadata = json.loads(metadata_path.read_text(encoding="utf-8"))

        self.assertEqual(len(dataset.columns), 2)
        self.assertEqual(set(dataset.run_ids.values()), {"run", "different-run"})
        self.assertNotIn("source_revision", metadata)
        self.assertEqual(
            {
                (source["platform"], source["compiler"], source["precision"])
                for source in metadata["sources"]
            },
            {
                ("windows", "MSVC", "f128"),
                ("wasm32", "Wasm32", "f256"),
            },
        )
        self.assertEqual(
            build_performance.operations(dataset),
            [
                ("arithmetic", "add"),
                ("floating_point_utilities", "abs"),
            ],
        )
        svg = build_performance.render(dataset)
        for text in (
            "bl::f128",
            "bl::f256",
            "Windows",
            "WebAssembly",
            "MSVC",
            "Wasm32",
        ):
            self.assertIn(f">{text}<", svg)
        self.assertEqual(svg.count('class="precision-header"'), 2)
        self.assertEqual(svg.count('class="platform-header"'), 2)
        self.assertEqual(svg.count('class="compiler-header"'), 2)
        precision_cells = [
            (int(x), int(width))
            for x, width in re.findall(
                r'<rect x="(\d+)"[^>]+width="(\d+)"[^>]+'
                r'class="precision-header-cell"/>',
                svg,
            )
        ]
        self.assertEqual(len(precision_cells), 2)
        self.assertGreater(
            precision_cells[1][0],
            precision_cells[0][0] + precision_cells[0][1],
        )
        self.assertEqual(svg.count('class="result-line"'), 4)

    def test_performance_reports_keep_consumer_modes_separate(self) -> None:
        source, target = synthetic_dataset(("fltx",))
        strict_rows = [dict(source.canonical[
            (target, "f128", "arithmetic", "add", "fltx")
        ])]
        fastmath_rows = [dict(strict_rows[0])]
        fastmath_rows[0]["run_id"] = "fastmath-run"

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            write_canonical(root / "windows" / "MSVC_f128.csv", strict_rows)
            write_canonical(
                root / "windows" / "MSVC_f128_fastmath.csv",
                fastmath_rows,
            )
            strict = build_performance.discover(root)
            fastmath = build_performance.discover(root, "fastmath")
            svg_path, metadata_path = build_performance.build(
                root,
                root / "generated",
                "compact",
                "fastmath",
            )
            svg = svg_path.read_text(encoding="utf-8")
            metadata = json.loads(metadata_path.read_text(encoding="utf-8"))

        self.assertEqual(set(strict.run_ids.values()), {"run"})
        self.assertEqual(set(fastmath.run_ids.values()), {"fastmath-run"})
        self.assertEqual(svg_path.name, "performance_table_fastmath_compact.svg")
        self.assertIn("consumer fast-math performance", svg)
        self.assertEqual(metadata["consumer_mode"], "fastmath")

    def test_performance_discovery_rejects_a_malformed_present_csv(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "windows" / "MSVC_f128.csv"
            path.parent.mkdir(parents=True)
            path.write_text("wrong\nvalue\n", encoding="utf-8")
            with self.assertRaisesRegex(
                run_metrics.MetricsError,
                "incompatible schema",
            ):
                build_performance.discover(Path(temporary))

    def test_performance_keeps_a_neutral_fltx_only_result(self) -> None:
        dataset, _ = synthetic_performance_dataset(("fltx",))
        svg = build_performance.render(dataset)
        self.assertIn(">10.0ns<", svg)
        self.assertIn("No competitor benchmark is available.", svg)
        self.assertNotIn("× vs", svg)

    def test_compact_performance_keeps_coverage_while_reducing_size(self) -> None:
        dataset, _ = synthetic_performance_dataset(
            ("fltx", "cppdd", "qdpp"),
        )
        full = build_performance.render(dataset, "full")
        compact = build_performance.render(dataset, "compact")

        dimensions = lambda svg: tuple(
            int(value)
            for value in re.search(
                r'<svg[^>]+width="(\d+)" height="(\d+)"',
                svg,
            ).groups()
        )
        full_width, full_height = dimensions(full)
        compact_width, compact_height = dimensions(compact)
        self.assertLess(compact_width, full_width)
        self.assertLess(compact_height, full_height)
        self.assertEqual(
            full.count('class="operation-label"'),
            compact.count('class="operation-label"'),
        )
        self.assertIn(">0.8× vs ddreal<", compact)
        self.assertNotIn('font-size="24"', compact)

    def test_overview_has_one_body_row_per_operation_and_fltx_first(self) -> None:
        dataset, target = synthetic_dataset(("fltx", "qdpp", "cppdd", "tlfloat"))
        svg = build_overview.render_overview(dataset, target, "f128")
        self.assertEqual(svg.count('class="body-row"'), 1)
        self.assertLess(svg.find(">bl::f128<"), svg.find(">qdpp (dd_real)<"))
        self.assertLess(
            svg.find(">bl::f128<"),
            svg.find(">boost (cpp_double_double)<"),
        )
        self.assertIn(">TLFloat (tlquad)<", svg)
        self.assertIn("&lt;cppdd &amp; api&gt;", svg)

    def test_overview_uses_qualified_competitor_primary_headers(self) -> None:
        self.assertEqual(
            build_overview.PRIMARY_HEADER_LABELS,
            {
                "ddreal": "qdpp (dd_real)",
                "qdreal": "qdpp (qd_real)",
                "cppdd": "boost (cpp_double_double)",
                "mpfr64": "mpfr_float_backend<64>",
                "tlquad": "TLFloat (tlquad)",
                "tloct": "TLFloat (tloct)",
            },
        )

    def test_overview_formats_exact_unavailable_and_fixed_numbers(self) -> None:
        self.assertEqual(build_overview._fmt_bits("inf"), "exact")
        self.assertEqual(build_overview._fmt_bits(""), "-")
        self.assertEqual(build_overview._fmt_bits_compact("240.6"), "240.6")
        self.assertEqual(build_overview._fmt_bits_compact("215.4"), "215.4")
        self.assertEqual(build_overview._fmt_bits_compact("inf"), "=")
        self.assertEqual(build_overview._fmt_ns("8770"), "8,770")
        self.assertNotIn("e", build_overview._fmt_ns("8770").lower())
        self.assertEqual(build_overview._fmt_ns_compact("8770"), "8770")
        self.assertEqual(build_overview._fmt_ns_compact("361.1"), "361")
        self.assertEqual(build_overview._fmt_ns_compact("11.22"), "11.2")
        self.assertEqual(build_overview._fmt_ns_compact("1.70"), "1.7")
        self.assertEqual(build_overview._fmt_ns_compact("0.123"), "0.12")
        self.assertEqual(
            build_overview._benchmark_text(
                {"ns_iter": "8770", "speed_ratio": ""}, "fltx"
            ),
            "8,770 ns",
        )

    def test_overview_domain_colours_cover_pass_near_and_failure(self) -> None:
        row = {
            "domains_passed": "4", "domains_total": "4",
            "min_margin_bits": "8",
        }
        self.assertEqual(
            build_overview._domain_style(row, "#123456"),
            ("4/4", "#123456", "#4ade80"),
        )
        row["min_margin_bits"] = "2"
        self.assertEqual(build_overview._domain_style(row)[2], "#fbbf24")
        row["domains_passed"] = "3"
        self.assertEqual(build_overview._domain_style(row)[2], "#f87171")
        self.assertEqual(build_overview._domain_style(None)[0], "-")

    def test_ratio_green_scale_brightens_above_unchanged_one_x(self) -> None:
        self.assertEqual(build_tables._ratio_color(1.0), "#9BE885")

        def luminance(color: str) -> float:
            red, green, blue = (
                int(color[index:index + 2], 16)
                for index in (1, 3, 5)
            )
            return 0.2126 * red + 0.7152 * green + 0.0722 * blue

        green_scale = tuple(
            build_tables._ratio_color(ratio)
            for ratio in (1.0, 2.0, 3.0, 4.0, 8.0, 20.0)
        )
        self.assertTrue(all(
            luminance(left) < luminance(right)
            for left, right in zip(green_scale, green_scale[1:])
        ))
        self.assertEqual(green_scale[-1], "#70FF98")

    def test_overview_accepts_all_special_support_categories(self) -> None:
        for category in ("Both", "Inf", "NaN", "No", "-"):
            text, fill, foreground = build_overview._special_style(
                {"special_support": category},
                "#123456",
            )
            self.assertEqual(text, category)
            self.assertEqual(fill, "#123456")
            self.assertEqual(foreground, build_overview.SUPPORT[category])
        with self.assertRaises(run_metrics.MetricsError):
            build_overview._special_style({"special_support": "maybe"})

    def test_overview_shows_implementation_speed_relative_to_fltx(self) -> None:
        row = {"ns_iter": "348.1", "speed_ratio": "2.05"}
        text, fill, foreground = build_overview._benchmark_style(
            row, "cppdd", "#123456"
        )
        self.assertEqual(text, "348.1 ns · 0.49×")
        self.assertEqual(fill, "#123456")
        self.assertEqual(foreground, "#e7e9ee")
        self.assertEqual(
            build_overview._benchmark_spans(row, "cppdd"),
            (
                ("348.1 ns", "#e7e9ee"),
                (" · ", build_overview.BENCHMARK_SEPARATOR_TEXT),
                ("0.49×", build_tables._ratio_color(1 / 2.05)),
            ),
        )

    def test_overview_treats_fltx_as_baseline_not_a_self_comparison(self) -> None:
        row = {"ns_iter": "10", "speed_ratio": ""}
        text, fill, _ = build_overview._benchmark_style(
            row, "fltx", "#142434"
        )
        self.assertEqual(text, "10.00 ns")
        self.assertEqual(fill, "#142434")
        self.assertNotIn("×", text)

    def test_overview_colours_only_the_speed_ratio_text(self) -> None:
        dataset, target = synthetic_dataset(("fltx", "cppdd"))
        svg = build_overview.render_overview(dataset, target, "f128")
        ratio_color = build_tables._ratio_color(1 / 2)
        self.assertIn(
            '<tspan fill="#e7e9ee">20.00 ns</tspan>',
            svg,
        )
        self.assertIn(
            f'<tspan fill="{build_overview.BENCHMARK_SEPARATOR_TEXT}"> · </tspan>'
            f'<tspan fill="{ratio_color}">0.50×</tspan>',
            svg,
        )

    def test_compact_overview_hides_ratios_and_colours_timings(self) -> None:
        row = {"ns_iter": "348.1", "speed_ratio": "2.05"}
        self.assertEqual(
            build_overview._benchmark_text(row, "cppdd", "compact"),
            "348 ns",
        )
        self.assertEqual(
            build_overview._benchmark_spans(
                row,
                "cppdd",
                layout="compact",
            ),
            (
                ("348", build_tables._ratio_color(1 / 2.05)),
                (" ns", build_overview.COMPACT_BENCHMARK_UNIT_TEXT),
            ),
        )
        self.assertEqual(
            build_overview._benchmark_text(
                {"ns_iter": "10", "speed_ratio": ""},
                "fltx",
                "compact",
            ),
            "10.0 ns",
        )

        dataset, target = synthetic_dataset(("fltx", "cppdd"))
        full = build_overview.render_overview(
            dataset, target, "f128", "full",
        )
        compact = build_overview.render_overview(
            dataset, target, "f128", "compact",
        )
        self.assertIn(
            f'<tspan fill="{build_overview.BENCHMARK_SEPARATOR_TEXT}"> · </tspan>',
            full,
        )
        self.assertNotIn(
            f'<tspan fill="{build_overview.BENCHMARK_SEPARATOR_TEXT}"> · </tspan>',
            compact,
        )
        full_width = int(re.search(r'<svg[^>]+width="(\d+)"', full).group(1))
        compact_width = int(re.search(r'<svg[^>]+width="(\d+)"', compact).group(1))
        self.assertLess(compact_width, full_width)
        full_height = int(re.search(r'<svg[^>]+height="(\d+)"', full).group(1))
        compact_height = int(
            re.search(r'<svg[^>]+height="(\d+)"', compact).group(1)
        )
        self.assertEqual(
            compact_height,
            full_height
            - build_overview.TITLE_HEIGHT
            - build_overview.FOOTER_HEIGHT
            + 6,
        )
        self.assertNotIn("metrics overview", compact)
        self.assertNotIn("Accuracy is MPFR-relative", compact)
        self.assertNotIn("Text: green pass", compact)
        self.assertIn("metrics overview", full)
        self.assertIn("Text: green pass", full)
        self.assertIn("@font-face", compact)
        self.assertIn("font-family:'Ubuntu Mono'", compact)
        self.assertIn('font-size="11.8"', compact)
        self.assertNotIn("@font-face", full)
        self.assertIn(
            f'fill="{build_overview.OPERATION_ROW_FILLS[0]}"',
            compact,
        )
        for body_fill in (
            build_overview.FLTX_TINT[1],
            *(fill for _, fill in build_overview.IMPLEMENTATION_TINTS),
        ):
            self.assertNotIn(f'fill="{body_fill}"', compact)
        compact_fltx_header = compact.split(
            'data-implementation="fltx"', 1,
        )[1].split("</g>", 1)[0]
        compact_competitor_header = compact.split(
            'data-implementation="cppdd"', 1,
        )[1].split("</g>", 1)[0]
        self.assertIn(">bench<", compact_fltx_header)
        self.assertIn(">(ns)<", compact_fltx_header)
        self.assertIn(">(ns)<", compact_competitor_header)
        self.assertNotIn(">performance<", compact)
        self.assertNotIn(">time · speed vs FLTX<", compact)
        self.assertEqual(compact_fltx_header.count("<rect"), 8)
        self.assertRegex(
            compact_fltx_header,
            r'<rect[^>]+height="66" fill="#244b70"/>',
        )
        self.assertEqual(compact_competitor_header.count("<rect"), 8)
        self.assertRegex(
            compact_competitor_header,
            r'<rect[^>]+height="66" fill="#40354f"/>',
        )
        compact_operations = sorted({
            (group, operation)
            for (
                row_target,
                row_precision,
                group,
                operation,
                _,
            ) in dataset.canonical
            if row_target == target and row_precision == "f128"
        })
        compact_third_y = sum(build_overview.COMPACT_HEADER_HEIGHTS[:2])
        for implementation in ("fltx", "cppdd"):
            table_x = int(re.search(
                rf'data-implementation="{implementation}"[^>]+data-x="(\d+)"',
                compact,
            ).group(1))
            widths = build_overview._column_widths(
                dataset,
                target,
                "f128",
                compact_operations,
                implementation,
                "compact",
            )
            divider = (
                f'<line x1="{table_x}" y1="{compact_third_y}" '
                f'x2="{table_x + sum(widths[:3])}" y2="{compact_third_y}"'
            )
            self.assertIn(divider, compact)
            self.assertNotIn(
                (
                    f'<line x1="{table_x}" y1="{compact_third_y}" '
                    f'x2="{table_x + sum(widths[:4])}" '
                    f'y2="{compact_third_y}"'
                ),
                compact,
            )
        self.assertIn(">bits<", compact_fltx_header)
        self.assertIn(">accurate<", compact_fltx_header)
        self.assertIn(">avg<", compact_fltx_header)
        self.assertIn(">min<", compact_fltx_header)
        self.assertRegex(
            compact_fltx_header,
            r'font-size="13" font-weight="normal"[^>]*>bl::f128</text>',
        )
        self.assertRegex(
            compact,
            r'font-size="13" font-weight="bold">operation</text>',
        )
        self.assertRegex(
            compact,
            r'font-size="11\.8" font-weight="normal">Arithmetic</text>',
        )
        self.assertRegex(
            compact,
            r'font-size="11\.8" font-weight="normal">add</text>',
        )
        self.assertRegex(
            full.split(
                'data-implementation="fltx"', 1,
            )[1].split("</g>", 1)[0],
            r'font-size="12" font-weight="bold"[^>]*>bl::f128</text>',
        )
        self.assertRegex(
            full,
            r'font-size="12" font-weight="bold">operation</text>',
        )
        self.assertRegex(
            full,
            r'font-size="12" font-weight="normal">Arithmetic</text>',
        )
        self.assertRegex(
            full,
            r'font-size="12" font-weight="normal">add</text>',
        )
        competitor = dataset.canonical[
            (target, "f128", "arithmetic", "add", "cppdd")
        ]
        tooltip = build_overview._tooltip(
            dataset,
            target,
            "f128",
            "arithmetic",
            "add",
            "cppdd",
            competitor,
        )
        self.assertIn("benchmark: 20.00 ns/iteration", tooltip)

    def test_compact_overview_sizes_text_for_ubuntu_mono(self) -> None:
        self.assertEqual(build_overview.COMPACT_MARGIN, 0)
        self.assertEqual(build_overview.COMPACT_TABLE_GAP, 3)
        self.assertEqual(
            build_overview.COMPACT_HEADER_HEIGHTS,
            (
                build_overview.HEADER_HEIGHTS[0],
                build_overview.HEADER_HEIGHTS[1] + 6,
                build_overview.HEADER_HEIGHTS[2],
            ),
        )
        self.assertEqual(
            build_overview._block_x(
                0,
                100,
                (200,),
                margin=build_overview.COMPACT_MARGIN,
                table_gap=build_overview.COMPACT_TABLE_GAP,
            ),
            103,
        )
        self.assertAlmostEqual(
            build_overview._estimated_character_width("compact", 9),
            5.9,
        )
        self.assertAlmostEqual(
            build_overview._estimated_character_width("compact", 10),
            6.5,
        )
        self.assertEqual(
            build_overview._estimated_character_width("full", 9),
            (
                build_overview.MONOSPACE_CHARACTER_WIDTH
                * build_overview.FONT_SCALE
            ),
        )
        operations = [("group", "long_operation")]
        self.assertEqual(
            build_overview._operation_width(operations, "compact"),
            (
                math.ceil(
                    len("long_operation")
                    * build_overview._estimated_character_width("compact", 9)
                )
                + build_overview.CELL_PADDING
                + 4
            ),
        )

    def test_overview_judges_fltx_against_the_fastest_competitor(self) -> None:
        dataset, target = synthetic_dataset(("fltx", "cppdd", "qdpp"))
        fastest = build_overview._fastest_competitor(
            dataset, target, "f128", "arithmetic", "add",
        )
        self.assertIsNotNone(fastest)
        competitor, ratio = fastest
        self.assertEqual(competitor["implementation"], "qdpp")
        self.assertEqual(ratio, 0.8)
        self.assertEqual(
            build_overview._benchmark_spans(
                dataset.canonical[
                    (target, "f128", "arithmetic", "add", "fltx")
                ],
                "fltx",
                fltx_speed_ratio=ratio,
            ),
            (("10.00 ns", build_tables._ratio_color(0.8)),),
        )
        self.assertEqual(
            build_overview._benchmark_spans(
                dataset.canonical[
                    (target, "f128", "arithmetic", "add", "fltx")
                ],
                "fltx",
                fltx_speed_ratio=ratio,
                layout="compact",
            ),
            (
                ("10.0", build_tables._ratio_color(0.8)),
                (" ns", build_overview.COMPACT_BENCHMARK_UNIT_TEXT),
            ),
        )
        tooltip = build_overview._tooltip(
            dataset,
            target,
            "f128",
            "arithmetic",
            "add",
            "fltx",
            dataset.canonical[
                (target, "f128", "arithmetic", "add", "fltx")
            ],
        )
        self.assertIn(
            "speed vs fastest competitor ddreal (qdpp dd_real): 0.8×",
            tooltip,
        )

    def test_overview_colours_every_exact_accuracy_value_green(self) -> None:
        self.assertEqual(
            build_overview._accuracy_style("inf", "#123456"),
            ("exact", "#123456", build_overview.EXACT_TEXT),
        )
        self.assertEqual(
            build_overview._accuracy_style("100", "#123456"),
            ("100.0", "#123456", "#e7e9ee"),
        )
        self.assertEqual(
            build_overview._accuracy_style(
                "100.4",
                "#123456",
                "compact",
            ),
            ("100.4", "#123456", "#e7e9ee"),
        )
        self.assertEqual(
            build_overview._accuracy_style(
                "inf",
                "#123456",
                "compact",
            ),
            ("=", "#123456", build_overview.COMPACT_EXACT_TEXT),
        )
        self.assertEqual(
            build_overview._accuracy_style("", "#123456", "compact"),
            ("-", "#123456", "#e7e9ee"),
        )
        dataset, target = synthetic_dataset(("fltx",))
        exact = dataset.canonical[
            (target, "f128", "arithmetic", "add", "fltx")
        ]
        exact["mean_bits"] = "inf"
        exact["p01_bits"] = "inf"
        exact["worst_bits"] = "inf"
        compact = build_overview.render_overview(
            dataset, target, "f128", "compact",
        )
        self.assertEqual(
            compact.count(
                'font-size="17.8" font-weight="normal">=</text>'
            ),
            2,
        )
        parts = []
        build_overview._infinity_mark(parts, 14, 11)
        mark = "".join(parts)
        self.assertIn('class="exact-mark"', mark)
        self.assertIn('stroke="#6ee7a0"', mark)
        self.assertIn('stroke-width="1.1"', mark)
        self.assertIn('stroke-width="1.65"', mark)
        self.assertIn('stroke-width="1.45"', mark)
        self.assertIn(
            'transform="translate(14 11) scale(0.5625) translate(-14 -11)"',
            mark,
        )
        self.assertIn('M 14 11 ', mark)
        self.assertIn('C 3 6.5 3 15.5', mark)

    def test_overview_reordering_does_not_change_competitor_tints(self) -> None:
        before = ("qdpp", "cppdd", "fltx")
        after = ("fltx", "qdpp", "cppdd")
        self.assertEqual(
            build_overview._implementation_tint("qdpp", before),
            build_overview._implementation_tint("qdpp", after),
        )
        self.assertEqual(
            build_overview._implementation_tint("cppdd", before),
            build_overview._implementation_tint("cppdd", after),
        )
        self.assertEqual(
            build_overview._implementation_tint("fltx", after),
            ("#244b70", "#1c2023"),
        )
        self.assertEqual(
            build_overview._implementation_tint("qdpp", after),
            ("#40354f", "#1f1e22"),
        )
        body_tints = (
            build_overview.FLTX_TINT[1],
            *(body for _, body in build_overview.IMPLEMENTATION_TINTS),
        )
        for body in body_tints:
            channels = [int(body[index:index + 2], 16) for index in (1, 3, 5)]
            self.assertAlmostEqual(
                (min(channels) + max(channels)) / 2,
                29 * 1.1,
                delta=0.5,
            )
        source_tints = (
            "#202428",
            *(body for _, body in build_overview._IMPLEMENTATION_TINTS),
        )
        for source, normalized in zip(source_tints, body_tints):
            def saturation(color: str) -> float:
                channels = (
                    int(color[index:index + 2], 16) / 255
                    for index in (1, 3, 5)
                )
                return colorsys.rgb_to_hls(*channels)[2]

            self.assertAlmostEqual(
                saturation(normalized),
                saturation(source),
                delta=0.025,
            )
        self.assertEqual(
            build_overview.OPERATION_ROW_FILLS,
            ("#191d23", "#16191f"),
        )

    def test_overview_uses_gapped_subtables_and_clear_group_dividers(self) -> None:
        dataset, target = synthetic_dataset(("fltx", "qdpp", "cppdd"))
        for implementation in ("fltx", "qdpp", "cppdd"):
            source = dataset.canonical[
                (target, "f128", "arithmetic", "add", implementation)
            ]
            row = dict(source)
            row["group"] = "rounding"
            row["operation"] = "round"
            dataset.canonical[
                (target, "f128", "rounding", "round", implementation)
            ] = row
        svg = build_overview.render_overview(dataset, target, "f128")
        positions = [
            (int(x), int(width))
            for x, width in re.findall(
                r'class="implementation-table"[^>]+data-x="(\d+)" '
                r'data-width="(\d+)"',
                svg,
            )
        ]
        self.assertEqual(len(positions), 3)
        for (left_x, width), (right_x, _) in zip(positions, positions[1:]):
            self.assertEqual(
                right_x - (left_x + width),
                build_overview.TABLE_GAP,
            )
        self.assertEqual(svg.count('class="group-row"'), 2)
        self.assertIn(">Arithmetic<", svg)
        self.assertIn(">Rounding<", svg)
        self.assertIn(f'fill="{build_overview.GROUP_ROW_FILL}"', svg)
        self.assertEqual(build_overview.GRID_REGULAR, "#000000")
        self.assertEqual(build_overview.GRID_STRONG, "#000000")
        self.assertEqual(build_overview.GROUP_ROW_FILL, "#4e555f")
        self.assertIn('stroke="#000000"', svg)
        self.assertEqual(build_overview.GROUP_ROW_TEXT, "#ffffff")
        self.assertRegex(
            svg,
            r'fill="#ffffff"[^>]+font-weight="normal"[^>]*>Arithmetic</text>',
        )
        self.assertNotIn('stroke-width="2"', svg)
        self.assertNotIn('stroke-width="3"', svg)
        self.assertIn('stroke-width="1"', svg)
        self.assertIn(">performance<", svg)
        self.assertIn(">time · speed vs FLTX<", svg)
        self.assertIn("speed is implementation ÷ FLTX", svg)
        self.assertIn(">Inf/<", svg)
        fltx_header = svg.split(
            'data-implementation="fltx"', 1,
        )[1].split("</g>", 1)[0]
        qdpp_header = svg.split(
            'data-implementation="qdpp"', 1,
        )[1].split("</g>", 1)[0]
        self.assertIn(">performance<", fltx_header)
        self.assertIn(">time<", fltx_header)
        self.assertNotIn(">time · speed vs FLTX<", fltx_header)
        self.assertIn(">performance<", qdpp_header)
        self.assertIn(">time · speed vs FLTX<", qdpp_header)
        self.assertRegex(
            svg,
            r'<rect x="12" y="54"[^>]+height="84" fill="#2B2B2B"/>',
        )
        self.assertEqual(fltx_header.count("<rect"), 9)
        self.assertRegex(
            fltx_header,
            r'<rect[^>]+height="28" fill="#244b70"/>',
        )
        self.assertEqual(qdpp_header.count("<rect"), 9)
        self.assertEqual(build_overview.CELL_PADDING, 8)
        self.assertEqual(
            tuple(specification[2] for specification in build_overview.COLUMN_SPECS),
            (44, 44, 40, 40, 40),
        )
        def required(*lines: str) -> int:
            return (
                math.ceil(
                    max(map(len, lines), default=0)
                    * build_overview.MONOSPACE_CHARACTER_WIDTH
                    * build_overview.FONT_SCALE
                )
                + build_overview.CELL_PADDING
            )

        operation_rows = [
            ("arithmetic", "add"),
            ("rounding", "round"),
        ]
        fltx_widths = build_overview._column_widths(
            dataset, target, "f128", operation_rows, "fltx",
        )
        qdpp_widths = build_overview._column_widths(
            dataset, target, "f128", operation_rows, "qdpp",
        )
        self.assertGreaterEqual(
            fltx_widths[0] + fltx_widths[1], required("bits accurate")
        )
        self.assertGreaterEqual(
            qdpp_widths[0] + qdpp_widths[1], required("bits accurate")
        )
        for widths in (fltx_widths, qdpp_widths):
            self.assertGreaterEqual(widths[2], required("domain"))
            self.assertGreaterEqual(widths[3], required("performance"))
            self.assertGreaterEqual(widths[4], required("Inf/", "NaN"))
        self.assertGreaterEqual(fltx_widths[3], required("time"))
        self.assertGreaterEqual(
            qdpp_widths[3], required("time · speed vs FLTX")
        )
        self.assertLess(fltx_widths[3], qdpp_widths[3])
        fltx_x = positions[0][0]
        full_third_y = sum(build_overview.HEADER_HEIGHTS[:2]) + build_overview.TITLE_HEIGHT
        self.assertIn(
            (
                f'<line x1="{fltx_x}" y1="{full_third_y}" '
                f'x2="{fltx_x + sum(fltx_widths[:4])}" '
                f'y2="{full_third_y}"'
            ),
            svg,
        )

        compact_qdpp_widths = build_overview._column_widths(
            dataset,
            target,
            "f128",
            operation_rows,
            "qdpp",
            "compact",
        )
        compact_benchmark_values = [
            build_overview._benchmark_text(
                dataset.canonical[
                    (target, "f128", group, operation, "qdpp")
                ],
                "qdpp",
                "compact",
            )
            for group, operation in operation_rows
        ]
        compact_benchmark_content = (
            math.ceil(
                max(map(len, (*compact_benchmark_values, "(ns)")))
                * build_overview._estimated_character_width("compact", 9)
            )
            + build_overview.CELL_PADDING
        )
        self.assertEqual(
            compact_qdpp_widths[3],
            compact_benchmark_content,
        )
        self.assertEqual(compact_qdpp_widths[:2], (39, 33))


class ProfileComparisonTests(unittest.TestCase):
    @staticmethod
    def comparison(
        **fastmath_changes: str,
    ) -> tuple[
        build_profile_comparison.Comparison,
        build_tables.Target,
    ]:
        strict, target = synthetic_dataset(("fltx",))
        fastmath, _ = synthetic_dataset(("fltx",))
        fastmath_row = fastmath.canonical[
            (target, "f128", "arithmetic", "add", "fltx")
        ]
        fastmath_row.update(fastmath_changes)
        strict.runs[target] = "strict-run"
        fastmath.runs[target] = "fastmath-run"
        return (
            build_profile_comparison.Comparison(
                strict,
                fastmath,
                {"run_id": "strict-run"},
                {"run_id": "fastmath-run"},
                FINGERPRINT,
            ),
            target,
        )

    @staticmethod
    def metadata(mode: str, run_id: str) -> dict[str, object]:
        configuration = ValidationTests.configuration_banner()
        configuration["consumer"]["fast-math"] = (
            "on" if mode == "fastmath" else "off"
        )
        return {
            "schema_version": run_metrics.SCHEMA_VERSION,
            "run_id": run_id,
            "source_revision": f"revision-{run_id}",
            "source_fingerprint": FINGERPRINT,
            "platform": "windows",
            "compiler": "MSVC",
            "precisions": list(run_metrics.PRECISIONS),
            "implementations": {},
            "sample_mode": "full",
            "consumer_mode": mode,
            "accuracy_samples": 65536,
            "benchmark_samples": {"f128": 81920, "f256": 40960},
            "benchmark_trials": 7,
            "benchmark_minimum_trial_ms": 25,
            "phases_requested": ["accuracy", "benchmark"],
            "host": {"processor": "test-cpu", "os": "Windows"},
            "executables": {},
            "created_utc": run_id,
            "status": "complete",
            "phases": {},
            "outputs": {},
            "configuration": configuration,
        }

    def test_comparison_formats_improvements_regressions_and_exact_matches(self) -> None:
        self.assertEqual(
            build_profile_comparison._accuracy_spans("100", "101.25"),
            (
                ("100.0", build_profile_comparison.NEUTRAL_TEXT),
                (" (+1.2)", build_profile_comparison.IMPROVEMENT_TEXT),
            ),
        )
        self.assertEqual(
            build_profile_comparison._accuracy_spans("inf", "219"),
            (
                ("exact", build_overview.EXACT_TEXT),
                (" (\u2192 219.0)", build_profile_comparison.REGRESSION_TEXT),
            ),
        )
        self.assertEqual(
            build_profile_comparison._accuracy_spans("219", "inf"),
            (
                ("219.0", build_profile_comparison.NEUTRAL_TEXT),
                (" (\u2192 exact)", build_profile_comparison.IMPROVEMENT_TEXT),
            ),
        )
        self.assertEqual(
            build_profile_comparison._accuracy_spans("inf", "inf"),
            (("exact", build_overview.EXACT_TEXT),),
        )
        self.assertEqual(
            build_profile_comparison._accuracy_spans("100", "100"),
            (("100.0", build_profile_comparison.NEUTRAL_TEXT),),
        )
        tiny = build_profile_comparison._accuracy_spans("100", "100.0001")
        self.assertEqual(tiny[1][0], " (+0.0001)")

        faster = build_profile_comparison._performance_spans("100", "90")
        slower = build_profile_comparison._performance_spans("100", "110")
        self.assertEqual(faster[-1][1], build_profile_comparison.IMPROVEMENT_TEXT)
        self.assertEqual(slower[-1][1], build_profile_comparison.REGRESSION_TEXT)
        self.assertIn("-10.00 ns", faster[-1][0])
        self.assertIn("+10.00 ns", slower[-1][0])
        self.assertEqual(
            build_profile_comparison._performance_spans("100", "100"),
            (("100.0 ns", build_profile_comparison.NEUTRAL_TEXT),),
        )

    def test_comparison_formats_domain_and_special_support_changes(self) -> None:
        strict = {
            "domains_passed": "3",
            "domains_total": "3",
            "min_margin_bits": "10",
        }
        fastmath = {**strict, "domains_passed": "2"}
        self.assertEqual(
            build_profile_comparison._domain_spans(strict, fastmath),
            (
                ("3/3", "#4ade80"),
                (" (-1)", build_profile_comparison.REGRESSION_TEXT),
            ),
        )
        self.assertEqual(
            build_profile_comparison._special_spans("Both", "Inf"),
            (
                ("Both", build_overview.SUPPORT["Both"]),
                (" (", build_profile_comparison.NEUTRAL_TEXT),
                ("-NaN", build_profile_comparison.REGRESSION_TEXT),
                (")", build_profile_comparison.NEUTRAL_TEXT),
            ),
        )
        swapped = build_profile_comparison._special_spans("Inf", "NaN")
        self.assertIn(("-Inf", build_profile_comparison.REGRESSION_TEXT), swapped)
        self.assertIn(("+NaN", build_profile_comparison.IMPROVEMENT_TEXT), swapped)
        self.assertEqual(
            build_profile_comparison._special_spans("Both", "Both"),
            (("Both", build_overview.SUPPORT["Both"]),),
        )

    def test_comparison_metadata_allows_separate_compatible_runs(self) -> None:
        strict = self.metadata("strict", "strict-run")
        fastmath = self.metadata("fastmath", "fastmath-run")
        self.assertIsNone(
            build_profile_comparison._compatibility_reason(strict, fastmath)
        )

        different_host = copy.deepcopy(fastmath)
        different_host["host"]["processor"] = "other-cpu"
        self.assertIn(
            "environments",
            build_profile_comparison._compatibility_reason(
                strict, different_host,
            ),
        )
        different_profile = copy.deepcopy(fastmath)
        different_profile["accuracy_samples"] = 4096
        self.assertIn(
            "sample profiles",
            build_profile_comparison._compatibility_reason(
                strict, different_profile,
            ),
        )
        different_source = copy.deepcopy(fastmath)
        different_source["source_fingerprint"] = "b" * 64
        self.assertIn(
            "fingerprints",
            build_profile_comparison._compatibility_reason(
                strict, different_source,
            ),
        )

    def test_comparison_rejects_row_shape_mismatches(self) -> None:
        comparison, target = self.comparison(domains_total="2")
        with self.assertRaisesRegex(run_metrics.MetricsError, "domain totals"):
            build_profile_comparison.render_comparison(
                comparison, target, "f128",
            )

    def test_comparison_renders_one_fltx_block_with_colored_deltas(self) -> None:
        comparison, target = self.comparison(ns_iter="9", mean_bits="109")
        svg = build_profile_comparison.render_comparison(
            comparison, target, "f128",
        )
        self.assertIn("f128 strict vs consumer fast-math", svg)
        self.assertEqual(svg.count(">bl::f128</text>"), 1)
        self.assertIn(">100.0</tspan>", svg)
        self.assertIn("> (-1.0)</tspan>", svg)
        self.assertIn("> (-1.00 ns)</tspan>", svg)
        self.assertIn(f'fill="{build_profile_comparison.REGRESSION_TEXT}"', svg)
        self.assertIn(f'fill="{build_profile_comparison.IMPROVEMENT_TEXT}"', svg)
        self.assertIn("strict run: strict-run", svg)
        self.assertIn("fast-math run: fastmath-run", svg)
        self.assertIn(FINGERPRINT, svg)

    def test_optional_comparison_clears_stale_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            target = build_tables.Target("windows", "MSVC")
            output = root / "generated" / "profile_comparison"
            paths = build_profile_comparison.output_paths(output, target)
            output.mkdir(parents=True)
            for path in paths:
                path.write_text("stale", encoding="utf-8")

            result = build_profile_comparison.build(
                root / "data", output, target,
            )

            self.assertFalse(result.outputs)
            self.assertIsNotNone(result.skipped)
            self.assertTrue(all(not path.exists() for path in paths))
            with self.assertRaisesRegex(run_metrics.MetricsError, "incomplete"):
                build_profile_comparison.build(
                    root / "data", output, target, required=True,
                )


class PresetPipelineTests(unittest.TestCase):
    def test_report_rebuild_uses_matching_category_layouts(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / "generated"
            target = build_tables.Target("windows", "MSVC")

            def build_accuracy(_input, _targets, destination, _consumer_mode):
                path = destination / "accuracy_table.svg"
                path.parent.mkdir(parents=True)
                path.write_text("<svg/>", encoding="utf-8")
                return [path]

            def build_performance(_input, destination, layout, _consumer_mode):
                suffix = "_compact" if layout == "compact" else ""
                svg = destination / f"performance_table{suffix}.svg"
                metadata = destination / f"performance_run{suffix}.json"
                destination.mkdir(parents=True, exist_ok=True)
                svg.write_text("<svg/>", encoding="utf-8")
                metadata.write_text("{}", encoding="utf-8")
                return svg, metadata

            with (
                mock.patch.object(
                    report_pipeline.build_tables,
                    "build",
                    side_effect=build_accuracy,
                ),
                mock.patch.object(
                    report_pipeline.build_performance,
                    "build",
                    side_effect=build_performance,
                ),
                mock.patch.object(
                    report_pipeline.build_tables,
                    "load",
                    return_value=mock.sentinel.dataset,
                ),
                mock.patch.object(
                    report_pipeline.build_overview,
                    "render_overview",
                    return_value="<svg/>",
                ),
            ):
                outputs = report_pipeline.rebuild(root / "data", output, (target,))

        self.assertEqual(len(outputs), 9)
        self.assertEqual(
            {path.parent.name for path in outputs},
            {"accuracy", "performance", "overview"},
        )
        self.assertIn(
            "windows_MSVC_f128_overview_compact.svg",
            {path.name for path in outputs},
        )

    def test_pipeline_defaults_to_strict_and_accepts_all(self) -> None:
        strict = preset_pipeline.parse_args(["--preset", "native-release"])
        all_modes = preset_pipeline.parse_args([
            "--preset",
            "native-release",
            "--consumer-mode",
            "all",
        ])

        self.assertEqual(strict.consumer_mode, "strict")
        self.assertEqual(all_modes.consumer_mode, "all")

    def test_pipeline_all_generates_both_local_layouts(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            input_root = root / "evidence"
            commands: list[list[str]] = []
            environments: list[dict[str, str]] = []
            selection = preset_pipeline.PresetSelection(
                build_name="mingw-release",
                configure_name="mingw-release",
                binary_dir=root / "build",
                configuration=None,
                generator="Ninja",
                compiler_hint="g++.exe",
                target_hint="x64-mingw-static",
                environment={"FLTX_PRESET_ENV": "active"},
            )

            def fake_run(command, command_root, environment=None) -> None:
                commands.append(list(command))
                environments.append(dict(environment or {}))
                script = Path(command[1]).name
                if script == "run_metrics.py":
                    consumer_mode = command[command.index("--consumer-mode") + 1]
                    handoff = Path(command[command.index("--result-file") + 1])
                    handoff.parent.mkdir(parents=True, exist_ok=True)
                    handoff.write_text(
                        json.dumps({
                            "schema_version": run_metrics.SCHEMA_VERSION,
                            "run_id": "run",
                            "sample_mode": "standard",
                            "consumer_mode": consumer_mode,
                            "platform": "windows",
                            "compiler": "MSVC",
                            "target": "windows/MSVC",
                            "input_root": str(input_root),
                            "metadata": "metadata.json",
                        }),
                        encoding="utf-8",
                    )
                    return

                if script == "build_profile_comparison.py":
                    self.assertIn("--required", command)
                    output = Path(command[command.index("--output") + 1])
                    for precision in run_metrics.PRECISIONS:
                        path = (
                            output
                            / f"windows_MSVC_{precision}_strict_fastmath_comp.svg"
                        )
                        path.parent.mkdir(parents=True, exist_ok=True)
                        path.write_text("<svg/>", encoding="utf-8")
                    return

                self.assertEqual(script, "rebuild_tables.py")
                output = Path(command[command.index("--output") + 1])
                consumer_mode = command[command.index("--consumer-mode") + 1]
                mode_suffix = run_metrics.consumer_mode_suffix(consumer_mode)
                generated = (
                    output / "accuracy" / f"accuracy_table{mode_suffix}.svg",
                    output / "performance" / f"performance_table{mode_suffix}.svg",
                    output / "performance" / f"performance_table{mode_suffix}_compact.svg",
                    output / "overview" / f"windows_MSVC_f128{mode_suffix}_overview.svg",
                    output / "overview" / f"windows_MSVC_f128{mode_suffix}_overview_compact.svg",
                    output / "overview" / f"windows_MSVC_f256{mode_suffix}_overview.svg",
                    output / "overview" / f"windows_MSVC_f256{mode_suffix}_overview_compact.svg",
                )
                for path in generated:
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text("<svg/>", encoding="utf-8")

            with (
                mock.patch.object(
                    preset_pipeline,
                    "resolve_preset",
                    return_value=selection,
                ),
                mock.patch.object(
                    preset_pipeline,
                    "existing_runner_artifacts",
                    return_value={
                        "fltx_accuracy": Path("accuracy.exe"),
                        "fltx_benchmark": Path("benchmark.exe"),
                        "fltx_accuracy_fastmath": Path("accuracy-fastmath.exe"),
                        "fltx_benchmark_fastmath": Path("benchmark-fastmath.exe"),
                    },
                ),
                mock.patch.object(
                    preset_pipeline,
                    "_run",
                    side_effect=fake_run,
                ),
            ):
                outputs = preset_pipeline.run_pipeline(
                    root,
                    "native-release",
                    consumer_mode="all",
                )

        metrics_command = commands[0]
        self.assertEqual(
            metrics_command[metrics_command.index("--sample-mode") + 1],
            "standard",
        )
        self.assertEqual(
            Path(metrics_command[metrics_command.index("--output-root") + 1]),
            root / "build" / "metrics" / "data",
        )
        self.assertEqual(len(commands), 5)
        self.assertEqual(
            environments,
            [dict(selection.environment)] * len(commands),
        )
        self.assertEqual(
            [Path(command[1]).name for command in commands],
            [
                "run_metrics.py",
                "rebuild_tables.py",
                "run_metrics.py",
                "rebuild_tables.py",
                "build_profile_comparison.py",
            ],
        )
        self.assertEqual(
            [
                command[command.index("--consumer-mode") + 1]
                for command in commands[:4]
            ],
            ["strict", "strict", "fastmath", "fastmath"],
        )
        self.assertEqual(len(outputs), 16)
        self.assertTrue(all(
            root / "build" / "metrics" / "generated" in path.parents
            for path in outputs
        ))

    def test_metrics_fail_fast_gates_all_accuracy_before_benchmarks(self) -> None:
        configuration = ValidationTests.configuration_banner()
        identity = source_fingerprint.SourceIdentity(
            revision="test-revision",
            fingerprint=FINGERPRINT,
        )
        with (
            mock.patch.object(
                run_metrics,
                "source_identity",
                return_value=identity,
            ),
            mock.patch.object(
                run_metrics,
                "_run_provenance",
                return_value={"host": {}, "executables": {}},
            ),
            mock.patch.object(
                run_metrics,
                "_preflight_runners",
                return_value=configuration,
            ),
            mock.patch.object(
                run_metrics,
                "_accept_runner_configuration",
                return_value=configuration,
            ),
            mock.patch.object(run_metrics, "_write_json"),
            mock.patch.object(
                run_metrics,
                "_run",
                side_effect=[
                    (configuration, 0),
                    (configuration, 1),
                ],
            ) as run,
            redirect_stderr(io.StringIO()),
        ):
            status = run_metrics.main([
                "--accuracy", "accuracy.exe",
                "--benchmark", "benchmark.exe",
                "--platform", "windows",
                "--compiler", "MSVC",
                "--sample-mode", "smoke",
            ])

        self.assertEqual(status, 1)
        self.assertEqual(run.call_count, 2)
        first_command = run.call_args_list[0].args[0]
        second_command = run.call_args_list[1].args[0]
        self.assertEqual(
            first_command[first_command.index("--precision") + 1],
            "f128",
        )
        self.assertEqual(
            second_command[second_command.index("--precision") + 1],
            "f256",
        )
        self.assertTrue(all(call.args[1] == "accuracy" for call in run.call_args_list))

    def test_standard_runs_both_accuracy_precisions_before_benchmarks(self) -> None:
        configuration = ValidationTests.configuration_banner()
        identity = source_fingerprint.SourceIdentity(
            revision="test-revision",
            fingerprint=FINGERPRINT,
        )
        lock = threading.Lock()
        accuracy_started: set[str] = set()
        both_accuracy_started = threading.Event()
        benchmark_saw_both = False

        def fake_run(command, runner_name, **kwargs):
            nonlocal benchmark_saw_both
            precision = command[command.index("--precision") + 1]
            if runner_name == "accuracy":
                with lock:
                    accuracy_started.add(precision)
                    if len(accuracy_started) == 2:
                        both_accuracy_started.set()
                both_accuracy_started.wait(timeout=2)
                return configuration, 0
            benchmark_saw_both = both_accuracy_started.is_set()
            return configuration, 1

        with (
            mock.patch.object(run_metrics, "source_identity", return_value=identity),
            mock.patch.object(
                run_metrics,
                "_run_provenance",
                return_value={"host": {}, "executables": {}},
            ),
            mock.patch.object(
                run_metrics, "_preflight_runners", return_value=configuration,
            ),
            mock.patch.object(
                run_metrics,
                "_accept_runner_configuration",
                return_value=configuration,
            ),
            mock.patch.object(run_metrics, "_write_json"),
            mock.patch.object(run_metrics, "_run", side_effect=fake_run) as run,
            redirect_stderr(io.StringIO()),
        ):
            status = run_metrics.main([
                "--accuracy", "accuracy.exe",
                "--benchmark", "benchmark.exe",
                "--platform", "windows",
                "--compiler", "MSVC",
                "--sample-mode", "standard",
            ])

        self.assertEqual(status, 1)
        self.assertEqual(accuracy_started, {"f128", "f256"})
        self.assertTrue(benchmark_saw_both)
        self.assertEqual(
            [call.args[1] for call in run.call_args_list].count("benchmark"),
            1,
        )

    def test_standard_accuracy_failure_prevents_all_benchmarks(self) -> None:
        configuration = ValidationTests.configuration_banner()
        identity = source_fingerprint.SourceIdentity(
            revision="test-revision",
            fingerprint=FINGERPRINT,
        )

        def fake_run(command, runner_name, **kwargs):
            precision = command[command.index("--precision") + 1]
            return configuration, int(runner_name == "accuracy" and precision == "f128")

        with (
            mock.patch.object(run_metrics, "source_identity", return_value=identity),
            mock.patch.object(
                run_metrics,
                "_run_provenance",
                return_value={"host": {}, "executables": {}},
            ),
            mock.patch.object(
                run_metrics, "_preflight_runners", return_value=configuration,
            ),
            mock.patch.object(
                run_metrics,
                "_accept_runner_configuration",
                return_value=configuration,
            ),
            mock.patch.object(run_metrics, "_write_json"),
            mock.patch.object(run_metrics, "_run", side_effect=fake_run) as run,
            redirect_stderr(io.StringIO()),
        ):
            status = run_metrics.main([
                "--accuracy", "accuracy.exe",
                "--benchmark", "benchmark.exe",
                "--platform", "windows",
                "--compiler", "MSVC",
                "--sample-mode", "standard",
            ])

        self.assertEqual(status, 1)
        self.assertEqual(run.call_count, 2)
        self.assertTrue(all(call.args[1] == "accuracy" for call in run.call_args_list))

    def test_existing_valid_runners_skip_cmake_preparation(self) -> None:
        selection = preset_pipeline.PresetSelection(
            build_name="native-release",
            configure_name="native-release",
            binary_dir=Path("build/native-release"),
            configuration="Release",
            generator="Ninja",
            compiler_hint="",
            target_hint="",
            environment={"FLTX_PRESET_ENV": "active"},
        )
        artifacts = {
            "fltx_accuracy": Path("accuracy.exe"),
            "fltx_benchmark": Path("benchmark.exe"),
            "fltx_accuracy_fastmath": Path("accuracy-fastmath.exe"),
            "fltx_benchmark_fastmath": Path("benchmark-fastmath.exe"),
        }
        with (
            mock.patch.object(
                preset_pipeline,
                "discover_runner_artifacts",
                return_value=artifacts,
            ) as discover,
            mock.patch.object(
                run_metrics,
                "validate_runner_artifacts",
                return_value=("windows", "MSVC"),
            ) as validate,
        ):
            observed = preset_pipeline.existing_runner_artifacts(
                Path("."),
                selection,
                "full",
                selection.environment,
            )
        self.assertIs(observed, artifacts)
        discover.assert_called_once_with(
            selection.binary_dir,
            selection.configuration,
        )
        self.assertEqual(validate.call_count, 2)
        validate.assert_has_calls([
            mock.call(
                Path("."),
                {
                    "fltx_accuracy": artifacts["fltx_accuracy"],
                    "fltx_benchmark": artifacts["fltx_benchmark"],
                },
                sample_mode="full",
                consumer_mode="strict",
                environment=selection.environment,
            ),
            mock.call(
                Path("."),
                {
                    "fltx_accuracy": artifacts["fltx_accuracy_fastmath"],
                    "fltx_benchmark": artifacts["fltx_benchmark_fastmath"],
                },
                sample_mode="full",
                consumer_mode="fastmath",
                environment=selection.environment,
            ),
        ])

    def test_stale_existing_runners_fall_back_to_cmake(self) -> None:
        selection = preset_pipeline.PresetSelection(
            build_name="native-release",
            configure_name="native-release",
            binary_dir=Path("build/native-release"),
            configuration="Release",
            generator="Ninja",
            compiler_hint="",
            target_hint="",
            environment={},
        )
        with (
            mock.patch.object(
                preset_pipeline,
                "discover_runner_artifacts",
                return_value={
                    "fltx_accuracy": Path("accuracy.exe"),
                    "fltx_benchmark": Path("benchmark.exe"),
                },
            ),
            mock.patch.object(
                run_metrics,
                "validate_runner_artifacts",
                side_effect=run_metrics.MetricsError("stale fingerprint"),
            ),
        ):
            observed = preset_pipeline.existing_runner_artifacts(
                Path("."),
                selection,
                "smoke",
            )
        self.assertIsNone(observed)

    def test_msvc_bootstrap_is_limited_to_native_windows_ninja(self) -> None:
        def selection(
            generator: str,
            *,
            compiler: str = "",
            target: str = "",
        ) -> preset_pipeline.PresetSelection:
            return preset_pipeline.PresetSelection(
                build_name="build",
                configure_name="configure",
                binary_dir=Path("build"),
                configuration="Release",
                generator=generator,
                compiler_hint=compiler,
                target_hint=target,
                environment={},
            )

        native = selection("Ninja")
        self.assertTrue(
            preset_pipeline.needs_msvc_x64_environment(
                native,
                host_system="Windows",
            )
        )
        self.assertFalse(
            preset_pipeline.needs_msvc_x64_environment(
                native,
                host_system="Linux",
            )
        )
        self.assertFalse(
            preset_pipeline.needs_msvc_x64_environment(
                selection("Visual Studio 18 2026"),
                host_system="Windows",
            )
        )
        self.assertFalse(
            preset_pipeline.needs_msvc_x64_environment(
                selection("Ninja", target="x64-mingw-static"),
                host_system="Windows",
            )
        )
        self.assertFalse(
            preset_pipeline.needs_msvc_x64_environment(
                selection("Ninja", compiler="clang++"),
                host_system="Windows",
            )
        )
        self.assertFalse(
            preset_pipeline.needs_msvc_x64_environment(
                selection("Ninja", target="wasm32-emscripten"),
                host_system="Windows",
            )
        )

    def test_resolves_included_and_inherited_build_preset(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "base-presets.json").write_text(
                json.dumps({
                    "version": 5,
                    "configurePresets": [{
                        "name": "base-config",
                        "generator": "Ninja",
                        "binaryDir": "${sourceDir}/out/$env{FLTX_TEST_OUTPUT}",
                        "environment": {"FLTX_TEST_OUTPUT": "release"},
                    }],
                    "buildPresets": [{
                        "name": "base-build",
                        "configurePreset": "release-config",
                        "configuration": "Release",
                    }],
                }),
                encoding="utf-8",
            )
            (root / "CMakePresets.json").write_text(
                json.dumps({
                    "version": 5,
                    "include": ["base-presets.json"],
                    "configurePresets": [{
                        "name": "release-config",
                        "inherits": "base-config",
                    }],
                    "buildPresets": [{
                        "name": "native-release",
                        "inherits": "base-build",
                    }],
                }),
                encoding="utf-8",
            )

            selection = preset_pipeline.resolve_preset(root, "native-release")
            self.assertEqual(selection.configure_name, "release-config")
            self.assertEqual(selection.configuration, "Release")
            self.assertEqual(selection.binary_dir, (root / "out" / "release").resolve())
            self.assertEqual(selection.environment["FLTX_TEST_OUTPUT"], "release")

    def test_mingw_configure_failure_explains_toolchain_requirement(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            selection = preset_pipeline.PresetSelection(
                build_name="mingw-release",
                configure_name="mingw-release",
                binary_dir=root / "build",
                configuration=None,
                generator="Ninja",
                compiler_hint="",
                target_hint="x64-mingw-static",
                environment={"FLTX_PRESET_ENV": "active"},
            )
            with (
                mock.patch.object(
                    preset_pipeline,
                    "resolve_preset",
                    return_value=selection,
                ),
                mock.patch.object(
                    preset_pipeline,
                    "existing_runner_artifacts",
                    return_value=None,
                ),
                mock.patch.object(preset_pipeline, "prepare_file_api_query"),
                mock.patch.object(
                    preset_pipeline,
                    "_run",
                    side_effect=preset_pipeline.PipelineError("configure failed"),
                ) as run,
            ):
                with self.assertRaises(preset_pipeline.PipelineError) as raised:
                    preset_pipeline.run_pipeline(root, "mingw-release")

        message = str(raised.exception)
        self.assertIn("configure failed", message)
        self.assertIn("requires a MinGW compiler and Ninja", message)
        self.assertIn("vcpkg triplet", message)
        run.assert_called_once_with(
            ["cmake", "--preset", "mingw-release"],
            root.resolve(),
            None,
        )

    def test_file_api_selects_configured_runner_artifacts(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            binary = Path(temporary)
            reply = binary / ".cmake" / "api" / "v1" / "reply"
            reply.mkdir(parents=True)
            targets = []
            for name in preset_pipeline.RUNNER_TARGETS:
                artifact = binary / "tests" / "Release" / f"{name}.exe"
                artifact.parent.mkdir(parents=True, exist_ok=True)
                artifact.write_text("", encoding="utf-8")
                target_json = f"target-{name}.json"
                (reply / target_json).write_text(
                    json.dumps({
                        "type": "EXECUTABLE",
                        "nameOnDisk": artifact.name,
                        "artifacts": [{
                            "path": artifact.relative_to(binary).as_posix(),
                        }],
                    }),
                    encoding="utf-8",
                )
                targets.append({"name": name, "jsonFile": target_json})
            codemodel_name = "codemodel-v2-test.json"
            (reply / codemodel_name).write_text(
                json.dumps({
                    "configurations": [
                        {"name": "Debug", "targets": []},
                        {"name": "Release", "targets": targets},
                    ],
                }),
                encoding="utf-8",
            )
            (reply / "index-test.json").write_text(
                json.dumps({
                    "reply": {
                        "client-fltx-metrics": {
                            "query.json": {
                                "responses": [{
                                    "kind": "codemodel",
                                    "jsonFile": codemodel_name,
                                }],
                            },
                        },
                    },
                }),
                encoding="utf-8",
            )

            artifacts = preset_pipeline.discover_runner_artifacts(
                binary,
                "Release",
            )
            self.assertEqual(set(artifacts), set(preset_pipeline.RUNNER_TARGETS))
            self.assertTrue(all(path.is_file() for path in artifacts.values()))

    def test_preset_output_roots_separate_local_and_release_evidence(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.assertEqual(
                preset_pipeline.metrics_root(root, False),
                root / "build" / "metrics",
            )
            self.assertEqual(
                preset_pipeline.metrics_root(root, True),
                root / "validation" / "metrics",
            )


if __name__ == "__main__":
    unittest.main()
