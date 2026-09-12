#!/usr/bin/env python3
"""Build target-specific accuracy SVG tables."""

from __future__ import annotations

import argparse
import csv
import json
import math
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable
from manifest import enabled_implementations, normalize_operations

from run_metrics import (
    ACCURACY_FIELDS,
    BENCHMARK_FIELDS,
    CANONICAL_FIELDS,
    CONSUMER_MODES,
    SCHEMA_VERSION,
    MetricsError,
    _format,
    _require_source_fingerprint,
    _safe_component,
    _validate_completeness,
    _validate_identity,
    _validate_implementations,
    aggregate,
    consumer_mode_suffix,
    metrics_stem,
    require_consumer_mode,
    run_metadata_stem,
)
from svg_table import Cell, Column, Row, Table, WHITE, write


PRECISIONS = ("dd", "qd")
MISSING = "#e5e7eb"
RESULT_RED = "#b91c1c"
RESULT_AMBER = "#92400e"
RESULT_LIGHT_GREEN = "#15803d"
RESULT_DARK_GREEN = "#047857"
RATIO_COLOR_STOPS: tuple[tuple[float, str], ...] = (
    (0.10, "#FF0000"),
    (0.45, "#FA5B11"),
    (0.55, "#F19A51"),
    (0.97, "#D7DA81"),
    (1.00, "#9BE885"),
    (2.00, "#90F086"),
    (3.00, "#86F58A"),
    (4.00, "#7CF98E"),
    (8.00, "#72FD92"),
    (20.00, "#70FF98"),
)
GROUP_ORDER = {
    name: index
    for index, name in enumerate((
        "arithmetic", "io", "rounding", "remainders",
        "floating_point_utilities", "roots_and_powers", "exponentials",
        "logarithms", "trigonometric", "hyperbolic", "inverse_hyperbolic",
        "special_functions", "comparisons", "random", "mixed_workloads",
    ))
}

GROUP_LABELS = {
    "arithmetic": "Arithmetic",
    "mixed_workloads": "Mixed Workloads",
    "io": "IO",
    "rounding": "Rounding",
    "remainders": "Remainders",
    "floating_point_utilities": "Float Utils",
    "roots_and_powers": "Roots & powers",
    "exponentials": "Exponentials",
    "logarithms": "Logarithms",
    "trigonometric": "Trigonometric",
    "hyperbolic": "Hyperbolic",
    "inverse_hyperbolic": "Hyperbolic",
    "special_functions": "Special",
    "comparisons": "Comparisons",
    "random": "Random",
}

_OPERATIONS_BY_GROUP = {
    "arithmetic": ("add", "subtract", "multiply", "divide"),
    "io": ("to_string", "to_chars", "parse"),
    "rounding": (
        "floor", "ceil", "trunc", "round", "roundeven", "lround", "llround",
        "round_decimals", "round_significant",
    ),
    "remainders": ("fmod", "remainder", "remquo"),
    "floating_point_utilities": (
        "abs", "fabs", "fma", "recip", "fmin", "fmax", "fdim",
        "copysign", "ldexp", "scalbn", "scalbln", "frexp", "modf", "ilogb",
        "logb", "nextafter", "nexttoward",
    ),
    "roots_and_powers": ("sqr", "sqrt", "cbrt", "hypot", "pow", "ipow"),
    "exponentials": ("exp", "exp2", "expm1"),
    "logarithms": ("log", "log2", "log10", "log1p"),
    "trigonometric": (
        "sin", "cos", "sincos", "tan", "asin", "acos", "atan", "atan2",
    ),
    "hyperbolic": ("sinh", "cosh", "tanh"),
    "inverse_hyperbolic": ("asinh", "acosh", "atanh"),
    "special_functions": ("erfc", "erf", "tgamma", "lgamma"),
    "comparisons": (
        "equal", "not_equal", "less", "greater", "less_equal", "greater_equal",
    ),
    "random": ("uniform_real", "normal"),
    "mixed_workloads": (
        "product_sum", "chained_arithmetic", "affine_trig", "mandelbrot",
        "matrix_vector_4x4", "horner_polynomial", "newton_root",
        "log_sum_exp", "haversine",
    ),
}
REPORT_EXCLUDED_OPERATIONS = frozenset({
    ("random", "mt19937_64"),
})
REPORT_GROUP_ALIASES = {
    "inverse_hyperbolic": "hyperbolic",
}


def _report_group(group: str) -> str:
    return REPORT_GROUP_ALIASES.get(group, group)


OPERATION_ORDER = {
    group: {operation: index for index, operation in enumerate(operations)}
    for group, operations in _OPERATIONS_BY_GROUP.items()
}

PLATFORM_LABELS = {
    "windows": "Windows",
    "webassembly": "WebAssembly",
    "linux": "Linux",
    "macos": "macOS",
}
ARCHITECTURE_LABELS = {
    "x86_64": "x64",
    "arm64": "ARM64",
    "wasm32": "wasm32",
}
COMPILER_LABEL_LINES = {
    "msvc": ("MSVC",),
    "clangcl": ("Clang", "clang-cl"),
    "mingw": ("GCC", "MinGW-w64"),
    "gcc": ("GCC",),
    "clang": ("Clang",),
    "appleclang": ("Apple Clang",),
    "emscripten": ("Clang", "Emscripten"),
}


def platform_label(value: str) -> str:
    return PLATFORM_LABELS.get(
        value.casefold(), value[:1].upper() + value[1:],
    )


def architecture_label(value: str) -> str:
    return ARCHITECTURE_LABELS.get(value.casefold(), value)


def compiler_label_lines(value: str) -> tuple[str, ...]:
    key = "".join(
        character for character in value.casefold() if character.isalnum()
    )
    return COMPILER_LABEL_LINES.get(key, (value,))


def compiler_label(value: str) -> str:
    lines = compiler_label_lines(value)
    return lines[0] if len(lines) == 1 else f"{lines[0]} ({lines[1]})"


@dataclass(frozen=True)
class Target:
    platform: str
    architecture: str
    compiler: str

    @property
    def label(self) -> str:
        return (
            f"{platform_label(self.platform)} / "
            f"{architecture_label(self.architecture)} / "
            f"{compiler_label(self.compiler)}"
        )

    @property
    def identity(self) -> str:
        return f"{self.platform}/{self.architecture}/{self.compiler}"


@dataclass
class Dataset:
    canonical: dict[tuple[Target, str, str, str, str], dict[str, str]]
    accuracy: dict[tuple[Target, str, str, str, str, str], dict[str, str]]
    implementations: dict[tuple[Target, str], tuple[str, ...]]
    revisions: set[str]
    fingerprints: set[str]
    runs: dict[Target, str]
    consumer_mode: str = "strict"
    operations: tuple[str, ...] = ()


def _target(value: str) -> Target:
    parts = value.replace("\\", "/").split("/")
    if len(parts) != 3 or not all(parts):
        raise argparse.ArgumentTypeError(
            "target must be PLATFORM/ARCHITECTURE/COMPILER"
        )
    try:
        return Target(
            _safe_component(parts[0], "platform"),
            _safe_component(parts[1], "architecture"),
            _safe_component(parts[2], "compiler"),
        )
    except MetricsError as error:
        raise argparse.ArgumentTypeError(str(error)) from error


def _read(path: Path, fields: tuple[str, ...], *, allow_empty: bool = False) -> list[dict[str, str]]:
    try:
        with path.open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream)
            if tuple(reader.fieldnames or ()) != fields:
                raise MetricsError(f"{path}: incompatible schema/header")
            rows = list(reader)
    except OSError as error:
        raise MetricsError(f"cannot read {path}: {error}") from error
    if not rows and not allow_empty:
        raise MetricsError(f"{path}: contains no rows")
    if any(None in row or any(value is None for value in row.values()) for row in rows):
        raise MetricsError(f"{path}: malformed row width")
    return rows


def _index(
    rows: Iterable[dict[str, str]],
    fields: tuple[str, ...],
    source: Path,
) -> dict[tuple[str, ...], dict[str, str]]:
    result: dict[tuple[str, ...], dict[str, str]] = {}
    for row in rows:
        key = tuple(row[field] for field in fields)
        if key in result:
            raise MetricsError(f"{source}: duplicate row {key}")
        result[key] = row
    return result


def _metadata(path: Path, target: Target) -> dict[str, object]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise MetricsError(f"cannot read {path}: {error}") from error
    required = {
        "schema_version", "run_id", "source_revision", "source_fingerprint",
        "platform", "architecture", "compiler", "precisions", "implementations", "status",
        "configuration",
    }
    if not isinstance(value, dict) or not required.issubset(value):
        raise MetricsError(f"{path}: incomplete metadata")
    if (
        value["schema_version"] != SCHEMA_VERSION
        or value["status"] != "complete"
        or value["platform"] != target.platform
        or value["architecture"] != target.architecture
        or value["compiler"] != target.compiler
        or set(value["precisions"]) != set(PRECISIONS)
    ):
        raise MetricsError(f"{path}: incompatible or incomplete run")
    return value


def load(
    input_root: Path,
    targets: Iterable[Target],
    consumer_mode: str = "strict",
) -> Dataset:
    targets = tuple(targets)
    if not targets or len(set(targets)) != len(targets):
        raise MetricsError("requested targets must be non-empty and unique")

    canonical: dict[tuple[Target, str, str, str, str], dict[str, str]] = {}
    accuracy: dict[tuple[Target, str, str, str, str, str], dict[str, str]] = {}
    implementations: dict[tuple[Target, str], tuple[str, ...]] = {}
    revisions: set[str] = set()
    fingerprints: set[str] = set()
    runs: dict[Target, str] = {}
    selections: set[tuple[str, ...]] = set()

    for target in targets:
        detail = input_root / target.platform / target.architecture / "detail"
        metadata_path = detail / (
            f"{run_metadata_stem(target.compiler, consumer_mode)}.json"
        )
        metadata = _metadata(metadata_path, target)
        try:
            operations = normalize_operations(metadata.get("operations", []))
        except (ValueError, TypeError) as error:
            raise MetricsError(f"{metadata_path}: invalid operation selection: {error}") from error
        selections.add(operations)
        configuration = require_consumer_mode(
            metadata["configuration"],
            consumer_mode,
            metadata_path,
        )
        run_id = str(metadata["run_id"])
        revision = str(metadata["source_revision"])
        fingerprint = str(metadata["source_fingerprint"])
        _require_source_fingerprint(configuration, fingerprint, metadata_path)
        if not run_id or not revision:
            raise MetricsError(f"{metadata_path}: empty run identity")
        revisions.add(revision)
        fingerprints.add(fingerprint)
        runs[target] = run_id

        for precision in PRECISIONS:
            stem = metrics_stem(target.compiler, precision, consumer_mode)
            canonical_path = (
                input_root / target.platform / target.architecture / f"{stem}.csv"
            )
            accuracy_path = detail / f"{stem}_accuracy.csv"
            benchmark_path = detail / f"{stem}_benchmark.csv"
            canonical_rows = _read(canonical_path, CANONICAL_FIELDS)
            accuracy_rows = _read(accuracy_path, ACCURACY_FIELDS, allow_empty=bool(operations))
            benchmark_rows = _read(benchmark_path, BENCHMARK_FIELDS)
            for rows, path in (
                (accuracy_rows, accuracy_path),
                (benchmark_rows, benchmark_path),
            ):
                _validate_identity(
                    rows, path, run_id=run_id, revision=revision,
                    fingerprint=fingerprint, precision=precision,
                )
            qdpp_enabled = configuration["harness"]["qdpp"] == "on"
            tlfloat_enabled = configuration["harness"]["tlfloat"] == "on"
            _validate_completeness(
                accuracy_rows, benchmark_rows, precision=precision,
                accuracy_path=accuracy_path, benchmark_path=benchmark_path,
                qdpp_enabled=qdpp_enabled,
                tlfloat_enabled=tlfloat_enabled,
                consumer_mode=consumer_mode,
                operations=operations,
            )
            _validate_implementations(
                benchmark_rows, precision=precision,
                benchmark_path=benchmark_path,
                qdpp_enabled=qdpp_enabled,
                tlfloat_enabled=tlfloat_enabled,
                consumer_mode=consumer_mode,
            )

            try:
                declared = tuple(
                    str(item["id"])
                    for item in metadata["implementations"][precision]
                )
            except (KeyError, TypeError) as error:
                raise MetricsError(
                    f"{metadata_path}: invalid implementation metadata"
                ) from error
            observed_implementations = {
                row["implementation"] for row in benchmark_rows
            }
            expected_order = tuple(
                item.id for item in enabled_implementations(
                    precision, qdpp=qdpp_enabled, tlfloat=tlfloat_enabled,
                )
                if item.id in observed_implementations
            )
            if declared != expected_order or not declared or declared[0] != "fltx":
                raise MetricsError(
                    f"{metadata_path}: implementation order does not match rows"
                )
            implementations[(target, precision)] = declared

            expected_rows = aggregate(
                accuracy_rows, benchmark_rows,
                accuracy_path=accuracy_path, benchmark_path=benchmark_path,
                run_id=run_id,
            )
            expected = {
                (row["group"], row["operation"], row["implementation"]): {
                    field: str(_format(row[field])) for field in CANONICAL_FIELDS
                }
                for row in expected_rows
            }
            observed = _index(
                canonical_rows,
                ("group", "operation", "implementation"),
                canonical_path,
            )
            if observed != expected:
                raise MetricsError(f"{canonical_path}: canonical summary is inconsistent")

            canonical.update({
                (target, precision, group, operation, implementation): row
                for (group, operation, implementation), row in observed.items()
            })
            accuracy.update({
                (
                    target, precision, group, operation, implementation, domain,
                ): row
                for (group, operation, implementation, domain), row in _index(
                    accuracy_rows,
                    ("group", "operation", "implementation", "domain"),
                    accuracy_path,
                ).items()
            })

    if len(selections) != 1:
        raise MetricsError("requested targets have different operation selections")
    if len(revisions) != 1 or len(fingerprints) != 1:
        raise MetricsError("requested targets were not built from the same source")
    return Dataset(
        canonical,
        accuracy,
        implementations,
        revisions,
        fingerprints,
        runs,
        consumer_mode,
        operations,
    )


def _mix_channel(a: int, b: int, t: float) -> int:
    return round(a + (b - a) * t)


def _mix_color(a: str, b: str, t: float) -> str:
    t = max(0.0, min(1.0, t))
    left = tuple(int(a[index:index + 2], 16) for index in (1, 3, 5))
    right = tuple(int(b[index:index + 2], 16) for index in (1, 3, 5))
    mixed = tuple(_mix_channel(x, y, t) for x, y in zip(left, right))
    return f"#{mixed[0]:02X}{mixed[1]:02X}{mixed[2]:02X}"


def _ratio_color_from_stops(
    ratio: float | None,
    stops: tuple[tuple[float, str], ...],
) -> str:
    if ratio is None or math.isnan(ratio):
        return "#FFFFFF"
    if ratio <= stops[0][0]:
        return stops[0][1]
    if ratio >= stops[-1][0] or math.isinf(ratio):
        return stops[-1][1]
    low_ratio, low_color = stops[0]
    for high_ratio, high_color in stops[1:]:
        if ratio <= high_ratio:
            t = (
                (math.log(ratio) - math.log(low_ratio))
                / (math.log(high_ratio) - math.log(low_ratio))
            )
            t = max(0.0, min(1.0, t))
            t = t * t * (3.0 - 2.0 * t)
            return _mix_color(low_color, high_color, t)
        low_ratio, low_color = high_ratio, high_color
    return stops[-1][1]


def _ratio_color(ratio: float | None) -> str:
    """Vivid ratio palette used for result text on dark reports."""

    return _ratio_color_from_stops(ratio, RATIO_COLOR_STOPS)


def _margin_text_color(margin: float, passed: bool) -> str:
    if not passed or margin < 0:
        return RESULT_RED
    if margin < 4:
        return RESULT_AMBER
    if margin <= 16:
        return RESULT_LIGHT_GREEN
    return RESULT_DARK_GREEN


def _fmt(value: float) -> str:
    return f"{value:.1f}"


def _columns(targets: tuple[Target, ...]) -> tuple[Column, ...]:
    return tuple(
        Column(target.label, precision)
        for precision in PRECISIONS
        for target in targets
    )


def _operations(dataset: Dataset) -> list[tuple[str, str]]:
    pairs = {
        (key[2], key[3])
        for key in dataset.canonical
        if (key[2], key[3]) not in REPORT_EXCLUDED_OPERATIONS
    }
    return sorted(
        pairs,
        key=lambda item: (
            GROUP_ORDER.get(item[0], len(GROUP_ORDER)),
            item[0],
            OPERATION_ORDER.get(item[0], {}).get(
                item[1], len(OPERATION_ORDER.get(item[0], {})),
            ),
            item[1],
        ),
    )


def accuracy_table(dataset: Dataset, targets: tuple[Target, ...]) -> Table:
    rows = []
    for group, operation in _operations(dataset):
        cells = []
        for precision in PRECISIONS:
            for target in targets:
                fltx = dataset.canonical.get(
                    (target, precision, group, operation, "fltx")
                )
                if fltx is None or not fltx["worst_bits"]:
                    cells.append(Cell(("—",), MISSING, tooltip="No accuracy result"))
                    continue
                passed = fltx["accuracy_pass"] == "yes"
                margin = float(fltx["min_margin_bits"])
                exact = math.isinf(float(fltx["worst_bits"]))
                lines = (
                    ("exact",)
                    if exact
                    else (
                        f"avg {_fmt(float(fltx['mean_bits']))}b",
                        f"min {_fmt(float(fltx['worst_bits']))}b",
                    )
                ) + (
                    f"{'✓' if passed else '!'} "
                    f"{fltx['domains_passed']}/{fltx['domains_total']} domains",
                )
                tooltip = [
                    f"{target.label}; {precision}; {group} / {operation}",
                    f"{fltx['implementation_label']} ({fltx['api']}): "
                    f"mean {fltx['mean_bits']} bits; p01 {fltx['p01_bits']} bits; "
                    f"worst {fltx['worst_bits']} bits; "
                    f"margin {fltx['min_margin_bits']} bits",
                ]
                cells.append(Cell(
                    lines, WHITE,
                    _margin_text_color(margin, passed),
                    tooltip="\n".join(tooltip),
                ))
        rows.append(Row(GROUP_LABELS.get(group, group), operation, tuple(cells)))
    return Table(
        (
            "fltx consumer fast-math accuracy consistency"
            if dataset.consumer_mode == "fastmath"
            else "fltx accuracy consistency"
        ),
        (
            "FLTX MPFR-relative average and minimum tested bits accurate by target "
            "with consumer fast-math enabled."
            if dataset.consumer_mode == "fastmath"
            else "FLTX MPFR-relative average and minimum tested bits accurate by target."
        ),
        _columns(targets),
        tuple(rows),
        (
            "✓ All FLTX domains pass · ! At least one FLTX domain failed · — Not applicable",
            "Competitor results remain available in the per-target overview and CSV data.",
        ),
    )


def build(
    input_root: Path,
    targets: tuple[Target, ...],
    output: Path,
    consumer_mode: str = "strict",
) -> list[Path]:
    dataset = load(input_root, targets, consumer_mode)
    suffix = consumer_mode_suffix(consumer_mode)
    path = output / f"accuracy_table{suffix}.svg"
    write(accuracy_table(dataset, targets), path)
    metadata = {
        "schema_version": SCHEMA_VERSION,
        "consumer_mode": consumer_mode,
        "source_revision": next(iter(dataset.revisions)),
        "source_fingerprint": next(iter(dataset.fingerprints)),
        "targets": [target.identity for target in targets],
        "run_ids": {
            target.identity: dataset.runs[target]
            for target in targets
        },
        "outputs": [path.name],
    }
    (output / f"tables_run{suffix}.json").write_text(
        json.dumps(metadata, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return [path]


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=Path(__file__).parents[1] / "data")
    parser.add_argument("--targets", type=_target, nargs="+", required=True)
    parser.add_argument("--output", type=Path, default=Path(__file__).parents[1] / "generated")
    parser.add_argument(
        "--consumer-mode",
        choices=CONSUMER_MODES,
        default="strict",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        outputs = build(
            args.input,
            tuple(args.targets),
            args.output,
            args.consumer_mode,
        )
    except (MetricsError, OSError, ValueError) as error:
        print(f"table build failed: {error}", file=sys.stderr)
        return 1
    for output in outputs:
        print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
