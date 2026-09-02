#!/usr/bin/env python3
"""Compare strict and consumer-fast-math FLTX metrics as overview-style SVGs."""

from __future__ import annotations

import argparse
import copy
import html
import math
import sys
from dataclasses import dataclass
from functools import partial
from pathlib import Path

import build_overview as overview
import build_tables
from run_metrics import MetricsError, PRECISIONS, metrics_stem, run_metadata_stem


IMPROVEMENT_TEXT = "#4ade80"
REGRESSION_TEXT = "#f87171"
NEUTRAL_TEXT = "#e7e9ee"
MUTED_TEXT = "#a5abb5"
COMPARISON_SUFFIX = "_strict_fastmath_comp.svg"
_RUN_SPECIFIC_METADATA = {
    "consumer_mode",
    "created_utc",
    "executables",
    "implementations",
    "outputs",
    "phases",
    "run_id",
    "source_revision",
}
_SPECIAL_MEMBERS = {
    "Both": frozenset(("Inf", "NaN")),
    "Inf": frozenset(("Inf",)),
    "NaN": frozenset(("NaN",)),
    "No": frozenset(),
    "-": frozenset(),
}


@dataclass(frozen=True)
class Comparison:
    strict: build_tables.Dataset
    fastmath: build_tables.Dataset
    strict_metadata: dict[str, object]
    fastmath_metadata: dict[str, object]
    fingerprint: str


@dataclass(frozen=True)
class BuildResult:
    outputs: tuple[Path, ...]
    skipped: str | None = None


def output_paths(output: Path, target: build_tables.Target) -> tuple[Path, ...]:
    return tuple(
        output
        / (
            f"{target.platform}_{target.architecture}_{target.compiler}_{precision}"
            f"{COMPARISON_SUFFIX}"
        )
        for precision in PRECISIONS
    )


def _evidence_paths(
    input_root: Path,
    target: build_tables.Target,
    consumer_mode: str,
) -> tuple[Path, ...]:
    detail = input_root / target.platform / target.architecture / "detail"
    paths = [
        detail / f"{run_metadata_stem(target.compiler, consumer_mode)}.json"
    ]
    for precision in PRECISIONS:
        stem = metrics_stem(target.compiler, precision, consumer_mode)
        paths.extend((
            input_root / target.platform / target.architecture / f"{stem}.csv",
            detail / f"{stem}_accuracy.csv",
            detail / f"{stem}_benchmark.csv",
        ))
    return tuple(paths)


def _normalized_metadata(metadata: dict[str, object]) -> dict[str, object]:
    normalized = copy.deepcopy(metadata)
    for field in _RUN_SPECIFIC_METADATA:
        normalized.pop(field, None)
    configuration = normalized.get("configuration")
    consumer = (
        configuration.get("consumer")
        if isinstance(configuration, dict)
        else None
    )
    if not isinstance(consumer, dict):
        raise MetricsError("comparison metadata has invalid consumer configuration")
    consumer.pop("fast-math", None)
    configuration.pop("harness", None)
    return normalized


def _compatibility_reason(
    strict: dict[str, object],
    fastmath: dict[str, object],
) -> str | None:
    strict_fingerprint = str(strict.get("source_fingerprint", ""))
    fastmath_fingerprint = str(fastmath.get("source_fingerprint", ""))
    if strict_fingerprint != fastmath_fingerprint:
        return "strict and fast-math source fingerprints differ"
    if _normalized_metadata(strict) != _normalized_metadata(fastmath):
        return "strict and fast-math run environments or sample profiles differ"
    return None


def _fltx_rows(
    dataset: build_tables.Dataset,
    target: build_tables.Target,
    precision: str,
) -> dict[tuple[str, str], dict[str, str]]:
    rows: dict[tuple[str, str], dict[str, str]] = {}
    for (
        row_target,
        row_precision,
        group,
        operation,
        implementation,
    ), row in dataset.canonical.items():
        if (
            row_target != target
            or row_precision != precision
            or implementation != "fltx"
            or (group, operation) in build_tables.REPORT_EXCLUDED_OPERATIONS
        ):
            continue
        display = overview._display_row(
            dataset,
            target,
            precision,
            group,
            operation,
            implementation,
            row,
            normal_only=True,
        )
        assert display is not None
        rows[(group, operation)] = display
    return rows


def _ordered_operations(
    rows: dict[tuple[str, str], dict[str, str]],
) -> list[tuple[str, str]]:
    return sorted(
        rows,
        key=lambda value: (
            build_tables.GROUP_ORDER.get(value[0], len(build_tables.GROUP_ORDER)),
            value[0],
            build_tables.OPERATION_ORDER.get(value[0], {}).get(
                value[1], len(build_tables.OPERATION_ORDER.get(value[0], {})),
            ),
            value[1],
        ),
    )


def _comparison_rows(
    comparison: Comparison,
    target: build_tables.Target,
    precision: str,
) -> tuple[
    dict[tuple[str, str], dict[str, str]],
    dict[tuple[str, str], dict[str, str]],
    list[tuple[str, str]],
]:
    strict = _fltx_rows(comparison.strict, target, precision)
    fastmath = _fltx_rows(comparison.fastmath, target, precision)
    if not strict:
        raise MetricsError(f"{target.label} {precision}: no strict FLTX operations")
    if set(strict) != set(fastmath):
        missing = sorted(set(strict) - set(fastmath))
        extra = sorted(set(fastmath) - set(strict))
        raise MetricsError(
            f"{target.label} {precision}: strict/fast-math operation mismatch "
            f"(missing {missing[:5]}, unexpected {extra[:5]})"
        )
    for key in strict:
        left = strict[key]
        right = fastmath[key]
        for field in ("mean_bits", "worst_bits", "domains_total", "ns_iter"):
            if bool(left[field]) != bool(right[field]):
                raise MetricsError(
                    f"{target.label} {precision} {key[1]}: "
                    f"strict/fast-math {field} availability differs"
                )
        if left["domains_total"] != right["domains_total"]:
            raise MetricsError(
                f"{target.label} {precision} {key[1]}: "
                "strict/fast-math domain totals differ"
            )
        if (
            left.get("special_support", "-") == "-"
        ) != (
            right.get("special_support", "-") == "-"
        ):
            raise MetricsError(
                f"{target.label} {precision} {key[1]}: "
                "strict/fast-math special-value applicability differs"
            )
        if (
            left.get("signed_zero_support", "-") == "-"
        ) != (
            right.get("signed_zero_support", "-") == "-"
        ):
            raise MetricsError(
                f"{target.label} {precision} {key[1]}: "
                "strict/fast-math signed-zero applicability differs"
            )
        if (
            left.get("subnormal_support", "-") == "-"
        ) != (
            right.get("subnormal_support", "-") == "-"
        ):
            raise MetricsError(
                f"{target.label} {precision} {key[1]}: "
                "strict/fast-math subnormal applicability differs"
            )
    return strict, fastmath, _ordered_operations(strict)


def load_comparison(
    input_root: Path,
    target: build_tables.Target,
) -> Comparison:
    detail = input_root / target.platform / target.architecture / "detail"
    strict_path = detail / f"{run_metadata_stem(target.compiler, 'strict')}.json"
    fastmath_path = detail / f"{run_metadata_stem(target.compiler, 'fastmath')}.json"
    strict_metadata = build_tables._metadata(strict_path, target)
    fastmath_metadata = build_tables._metadata(fastmath_path, target)
    strict = build_tables.load(input_root, (target,), "strict")
    fastmath = build_tables.load(input_root, (target,), "fastmath")
    reason = _compatibility_reason(strict_metadata, fastmath_metadata)
    if reason is not None:
        raise MetricsError(reason)
    fingerprint = str(strict_metadata["source_fingerprint"])
    comparison = Comparison(
        strict,
        fastmath,
        strict_metadata,
        fastmath_metadata,
        fingerprint,
    )
    for precision in PRECISIONS:
        _comparison_rows(comparison, target, precision)
    return comparison


def _small_delta(value: float) -> str:
    magnitude = abs(value)
    if magnitude >= 0.05:
        return f"{magnitude:.1f}"
    if magnitude >= 0.0005:
        return f"{magnitude:.3f}".rstrip("0").rstrip(".")
    return f"{magnitude:.3g}"


def _accuracy_spans(
    strict_value: str,
    fastmath_value: str,
) -> tuple[tuple[str, str], ...]:
    if not strict_value:
        return (("-", MUTED_TEXT),)
    strict = float(strict_value)
    fastmath = float(fastmath_value)
    strict_exact = math.isinf(strict)
    fastmath_exact = math.isinf(fastmath)
    base = overview._fmt_bits(strict_value)
    base_color = overview.EXACT_TEXT if strict_exact else NEUTRAL_TEXT
    if strict == fastmath:
        return ((base, base_color),)
    if strict_exact:
        return (
            (base, base_color),
            (f" (\u2192 {overview._fmt_bits(fastmath_value)})", REGRESSION_TEXT),
        )
    if fastmath_exact:
        return (
            (base, base_color),
            (" (\u2192 exact)", IMPROVEMENT_TEXT),
        )
    delta = fastmath - strict
    sign = "+" if delta > 0 else "-"
    color = IMPROVEMENT_TEXT if delta > 0 else REGRESSION_TEXT
    return (
        (base, base_color),
        (f" ({sign}{_small_delta(delta)})", color),
    )


def _domain_spans(
    strict_row: dict[str, str],
    fastmath_row: dict[str, str],
) -> tuple[tuple[str, str], ...]:
    base, _, base_color = overview._domain_style(strict_row)
    if base == "-":
        return ((base, base_color),)
    strict = int(strict_row["domains_passed"])
    fastmath = int(fastmath_row["domains_passed"])
    delta = fastmath - strict
    if delta == 0:
        return ((base, base_color),)
    sign = "+" if delta > 0 else "-"
    color = IMPROVEMENT_TEXT if delta > 0 else REGRESSION_TEXT
    return ((base, base_color), (f" ({sign}{abs(delta)})", color))


def _performance_spans(
    strict_value: str,
    fastmath_value: str,
) -> tuple[tuple[str, str], ...]:
    if not strict_value:
        return (("-", MUTED_TEXT),)
    strict = float(strict_value)
    fastmath = float(fastmath_value)
    base = f"{overview._fmt_ns(strict_value)} ns"
    if strict == fastmath:
        return ((base, NEUTRAL_TEXT),)
    delta = fastmath - strict
    sign = "+" if delta > 0 else "-"
    color = REGRESSION_TEXT if delta > 0 else IMPROVEMENT_TEXT
    return (
        (base, NEUTRAL_TEXT),
        (
            f" ({sign}{overview._fmt_ns(str(abs(delta)))} ns)",
            color,
        ),
    )


def _special_spans(
    strict_value: str,
    fastmath_value: str,
) -> tuple[tuple[str, str], ...]:
    if strict_value not in _SPECIAL_MEMBERS or fastmath_value not in _SPECIAL_MEMBERS:
        raise MetricsError("malformed special-value support category")
    spans: list[tuple[str, str]] = [
        (strict_value, overview.SUPPORT[strict_value])
    ]
    if strict_value == fastmath_value:
        return tuple(spans)
    removed = _SPECIAL_MEMBERS[strict_value] - _SPECIAL_MEMBERS[fastmath_value]
    added = _SPECIAL_MEMBERS[fastmath_value] - _SPECIAL_MEMBERS[strict_value]
    spans.append((" (", NEUTRAL_TEXT))
    changes: list[tuple[str, str]] = []
    for capability in ("Inf", "NaN"):
        if capability in removed:
            changes.append((f"-{capability}", REGRESSION_TEXT))
    for capability in ("Inf", "NaN"):
        if capability in added:
            changes.append((f"+{capability}", IMPROVEMENT_TEXT))
    for index, change in enumerate(changes):
        if index:
            spans.append((" ", NEUTRAL_TEXT))
        spans.append(change)
    spans.append((")", NEUTRAL_TEXT))
    return tuple(spans)


def _boolean_support_spans(
    strict_value: str,
    fastmath_value: str,
    label: str,
) -> tuple[tuple[str, str], ...]:
    symbols = {"yes": "✓", "no": "✗", "-": "-"}
    colors = {"yes": IMPROVEMENT_TEXT, "no": REGRESSION_TEXT, "-": MUTED_TEXT}
    if strict_value not in symbols or fastmath_value not in symbols:
        raise MetricsError(f"malformed {label} support category")
    spans = [(symbols[strict_value], colors[strict_value])]
    if strict_value != fastmath_value:
        spans.extend((
            (" (", NEUTRAL_TEXT),
            (symbols[fastmath_value], colors[fastmath_value]),
            (")", NEUTRAL_TEXT),
        ))
    return tuple(spans)


def _signed_zero_spans(
    strict_value: str,
    fastmath_value: str,
) -> tuple[tuple[str, str], ...]:
    return _boolean_support_spans(
        strict_value, fastmath_value, "signed-zero",
    )


def _subnormal_spans(
    strict_value: str,
    fastmath_value: str,
) -> tuple[tuple[str, str], ...]:
    return _boolean_support_spans(
        strict_value, fastmath_value, "subnormal",
    )


def _plain(spans: tuple[tuple[str, str], ...]) -> str:
    return "".join(text for text, _ in spans)


def _cell_spans(
    strict_row: dict[str, str],
    fastmath_row: dict[str, str],
) -> tuple[tuple[tuple[str, str], ...], ...]:
    return (
        _accuracy_spans(strict_row["mean_bits"], fastmath_row["mean_bits"]),
        _accuracy_spans(strict_row["worst_bits"], fastmath_row["worst_bits"]),
        _domain_spans(strict_row, fastmath_row),
        _performance_spans(strict_row["ns_iter"], fastmath_row["ns_iter"]),
        _special_spans(
            strict_row.get("special_support", "-"),
            fastmath_row.get("special_support", "-"),
        ),
        _signed_zero_spans(
            strict_row.get("signed_zero_support", "-"),
            fastmath_row.get("signed_zero_support", "-"),
        ),
        _subnormal_spans(
            strict_row.get("subnormal_support", "-"),
            fastmath_row.get("subnormal_support", "-"),
        ),
    )


def _column_widths(
    strict: dict[tuple[str, str], dict[str, str]],
    fastmath: dict[tuple[str, str], dict[str, str]],
    operations: list[tuple[str, str]],
) -> tuple[int, ...]:
    visible = {
        "mean": ["mean"],
        "worst": ["worst"],
        "pass": ["pass"],
        "benchmark": ["time"],
        "special": ["Inf/", "NaN"],
        "signed_zero": ["±0"],
        "subnormal": ["Subnorm"],
    }
    keys = tuple(visible)
    for operation in operations:
        values = _cell_spans(strict[operation], fastmath[operation])
        for key, spans in zip(keys, values):
            visible[key].append(_plain(spans))

    def required(lines: list[str] | tuple[str, ...]) -> int:
        return (
            math.ceil(
                max(map(len, lines), default=0)
                * overview._estimated_character_width("full", 9)
            )
            + overview.CELL_PADDING
        )

    widths = [
        max(minimum, required(visible[key]))
        for (key, _, minimum) in overview._column_specs("fastmath")
    ]
    accuracy_width = required(("bits accurate",))
    deficit = max(0, accuracy_width - widths[0] - widths[1])
    widths[0] += (deficit + 1) // 2
    widths[1] += deficit // 2
    widths[2] = max(widths[2], required(("domain",)))
    widths[3] = max(widths[3], required(("performance",)))
    widths[4] = max(widths[4], required(("Inf/", "NaN")))
    widths[5] = max(widths[5], required(("±0",)))
    widths[6] = max(widths[6], required(("Subnorm",)))
    return tuple(widths)


def _tooltip(
    comparison: Comparison,
    operation: str,
    strict: dict[str, str],
    fastmath: dict[str, str],
) -> str:
    return "\n".join((
        f"fltx {operation}",
        f"strict run: {comparison.strict_metadata['run_id']}",
        f"fast-math run: {comparison.fastmath_metadata['run_id']}",
        f"source fingerprint: {comparison.fingerprint}",
        f"mean bits: {strict['mean_bits'] or '-'} -> {fastmath['mean_bits'] or '-'}",
        f"worst bits: {strict['worst_bits'] or '-'} -> {fastmath['worst_bits'] or '-'}",
        (
            f"domains: {strict['domains_passed'] or '-'}/"
            f"{strict['domains_total'] or '-'} -> "
            f"{fastmath['domains_passed'] or '-'}/"
            f"{fastmath['domains_total'] or '-'}"
        ),
        f"time: {strict['ns_iter'] or '-'} ns -> {fastmath['ns_iter'] or '-'} ns",
        (
            f"special values: {strict.get('special_support', '-')} -> "
            f"{fastmath.get('special_support', '-')}"
        ),
        (
            f"signed zero: {strict.get('signed_zero_support', '-')} -> "
            f"{fastmath.get('signed_zero_support', '-')}"
        ),
        (
            f"subnormal support: {strict.get('subnormal_support', '-')} -> "
            f"{fastmath.get('subnormal_support', '-')}"
        ),
    ))


def render_comparison(
    comparison: Comparison,
    target: build_tables.Target,
    precision: str,
) -> str:
    strict, fastmath, operations = _comparison_rows(
        comparison, target, precision,
    )
    column_widths = _column_widths(strict, fastmath, operations)
    operation_width = overview._operation_width(operations, "full")
    block_width = sum(column_widths)
    width = (
        overview.MARGIN * 2
        + operation_width
        + overview.TABLE_GAP
        + block_width
    )
    header_height = sum(overview.HEADER_HEIGHTS)
    body_y = overview.TITLE_HEIGHT + header_height
    operation_groups = overview._grouped_operations(operations)
    body_height = (
        overview.ROW_HEIGHT * len(operations)
        + overview.GROUP_ROW_HEIGHT * len(operation_groups)
    )
    height = body_y + body_height + overview.FOOTER_HEIGHT
    parts = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        (
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" '
            f'height="{height}" viewBox="0 0 {width} {height}">'
        ),
        overview._font_css("full"),
        f'<rect width="{width}" height="{height}" fill="#0f1115"/>',
    ]
    add_text = partial(overview._text, parts)
    add_spans = partial(overview._text_spans, parts)
    add_lines = partial(overview._text_lines, parts)
    add_text(
        overview.MARGIN,
        23,
        (
            f"{overview.PRECISION_TYPE_NAMES[precision]} strict vs consumer "
            f"fast-math \u2014 {target.label}"
        ),
        anchor="start",
        weight="bold",
        size=15,
    )
    add_text(
        overview.MARGIN,
        42,
        (
            "Strict first; deltas are fast-math - strict. Accuracy excludes "
            "the separate subnormal domain."
        ),
        anchor="start",
        fill="#aeb4bf",
        size=10,
    )

    header_y = overview.TITLE_HEIGHT
    overview._rect(
        parts,
        overview.MARGIN,
        header_y,
        operation_width,
        header_height,
        "#2B2B2B",
    )
    add_text(
        overview.MARGIN + operation_width / 2,
        header_y + header_height / 2 + 4,
        "operation",
        weight="bold",
        size=10,
    )
    block_x = overview.MARGIN + operation_width + overview.TABLE_GAP
    header_fill = overview.FLTX_TINT[0]
    overview._rect(
        parts, block_x, header_y, block_width,
        overview.HEADER_HEIGHTS[0], header_fill,
    )
    add_text(
        block_x + block_width / 2,
        header_y + 16,
        f"bl::{overview.PRECISION_TYPE_NAMES[precision]}",
        weight="bold",
        fill="#ffffff",
        size=10,
    )

    second_y = header_y + overview.HEADER_HEIGHTS[0]
    groups = (
        (("bits accurate",), column_widths[0] + column_widths[1], False),
        (("domain",), column_widths[2], False),
        (("performance",), column_widths[3], False),
        (("Inf/", "NaN"), column_widths[4], True),
        (("±0",), column_widths[5], True),
        (("Subnorm",), column_widths[6], True),
    )
    group_x = block_x
    for lines, group_width, spans_subheader in groups:
        group_height = (
            overview.HEADER_HEIGHTS[1] + overview.HEADER_HEIGHTS[2]
            if spans_subheader
            else overview.HEADER_HEIGHTS[1]
        )
        overview._rect(
            parts, group_x, second_y, group_width, group_height, header_fill,
        )
        add_lines(
            group_x + group_width / 2,
            second_y,
            group_height,
            lines,
            size=9,
        )
        group_x += group_width

    third_y = second_y + overview.HEADER_HEIGHTS[1]
    column_x = block_x
    leaf_headers = (
        ("mean",), ("worst",), ("pass",), ("time",), (), (), (),
    )
    for lines, column_width in zip(leaf_headers, column_widths):
        if lines:
            overview._rect(
                parts,
                column_x,
                third_y,
                column_width,
                overview.HEADER_HEIGHTS[2],
                header_fill,
            )
            add_lines(
                column_x + column_width / 2,
                third_y,
                overview.HEADER_HEIGHTS[2],
                lines,
                size=9,
            )
        column_x += column_width

    group_spans: list[tuple[int, int, int]] = []
    y = body_y
    row_index = 0
    for report_group, group_operations in operation_groups:
        group_y = y
        for left, table_width in (
            (overview.MARGIN, operation_width),
            (block_x, block_width),
        ):
            overview._rect(
                parts,
                left,
                group_y,
                table_width,
                overview.GROUP_ROW_HEIGHT,
                overview.GROUP_ROW_FILL,
            )
        add_text(
            overview.MARGIN + 7,
            group_y + 13,
            build_tables.GROUP_LABELS.get(
                report_group,
                report_group.replace("_", " ").title(),
            ),
            anchor="start",
            fill=overview.GROUP_ROW_TEXT,
            size=10,
        )
        y += overview.GROUP_ROW_HEIGHT
        operations_y = y
        for group, operation in group_operations:
            body_fill = overview.OPERATION_ROW_FILLS[row_index % 2]
            overview._rect(
                parts,
                overview.MARGIN,
                y,
                operation_width,
                overview.ROW_HEIGHT,
                body_fill,
            )
            add_text(
                overview.MARGIN + 7,
                y + 15,
                operation,
                anchor="start",
                fill="#f1f2f4",
                size=10,
            )
            strict_row = strict[(group, operation)]
            fastmath_row = fastmath[(group, operation)]
            values = _cell_spans(strict_row, fastmath_row)
            tooltip = _tooltip(
                comparison, operation, strict_row, fastmath_row,
            )
            column_x = block_x
            for column_width, spans in zip(column_widths, values):
                parts.append("<g>")
                parts.append(f"<title>{html.escape(tooltip)}</title>")
                overview._rect(
                    parts,
                    column_x,
                    y,
                    column_width,
                    overview.ROW_HEIGHT,
                    body_fill,
                )
                add_spans(
                    column_x + column_width / 2,
                    y + 15,
                    spans,
                    size=9,
                )
                parts.append("</g>")
                column_x += column_width
            y += overview.ROW_HEIGHT
            row_index += 1
        group_spans.append((group_y, operations_y, y))

    grid_bottom = y
    for left, right in (
        (overview.MARGIN, overview.MARGIN + operation_width),
        (block_x, block_x + block_width),
    ):
        overview._line(parts, left, header_y, right, header_y, stroke=overview.GRID_STRONG)
        overview._line(parts, left, header_y, left, grid_bottom, stroke=overview.GRID_STRONG)
        overview._line(parts, right, header_y, right, grid_bottom, stroke=overview.GRID_STRONG)
        overview._line(parts, left, grid_bottom, right, grid_bottom, stroke=overview.GRID_STRONG)

    overview._line(
        parts,
        block_x,
        second_y,
        block_x + block_width,
        second_y,
        stroke=overview.GRID_REGULAR,
    )
    overview._line(
        parts,
        block_x,
        third_y,
        block_x + sum(column_widths[:4]),
        third_y,
        stroke=overview.GRID_REGULAR,
    )
    boundaries: list[int] = []
    position = block_x
    for column_width in column_widths[:-1]:
        position += column_width
        boundaries.append(position)
    overview._line(parts, boundaries[0], third_y, boundaries[0], body_y)
    for boundary in boundaries[1:]:
        overview._line(parts, boundary, second_y, boundary, body_y)

    for group_y, operations_y, operations_end in group_spans:
        for left, right in (
            (overview.MARGIN, overview.MARGIN + operation_width),
            (block_x, block_x + block_width),
        ):
            overview._line(parts, left, group_y, right, group_y, stroke=overview.GRID_STRONG)
            overview._line(
                parts, left, operations_y, right, operations_y,
                stroke=overview.GRID_STRONG,
            )
            operation_y = operations_y + overview.ROW_HEIGHT
            while operation_y < operations_end:
                overview._line(
                    parts, left, operation_y, right, operation_y,
                    stroke=overview.GRID_REGULAR,
                )
                operation_y += overview.ROW_HEIGHT
        for boundary in boundaries:
            overview._line(
                parts, boundary, operations_y, boundary, operations_end,
                stroke=overview.GRID_REGULAR,
            )

    add_text(
        overview.MARGIN,
        grid_bottom + 20,
        "Deltas: green improves under fast-math; red regresses.",
        anchor="start",
        fill="#aeb4bf",
        size=10,
    )
    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def _clear_outputs(paths: tuple[Path, ...]) -> None:
    for path in paths:
        try:
            path.unlink(missing_ok=True)
        except OSError as error:
            raise MetricsError(f"cannot remove stale comparison {path}: {error}") from error


def build(
    input_root: Path,
    output: Path,
    target: build_tables.Target,
    *,
    required: bool = False,
) -> BuildResult:
    paths = output_paths(output, target)
    evidence = (
        *_evidence_paths(input_root, target, "strict"),
        *_evidence_paths(input_root, target, "fastmath"),
    )
    missing = [path for path in evidence if not path.is_file()]
    if missing:
        reason = "paired strict/fast-math evidence is incomplete"
        if required:
            raise MetricsError(f"{reason}: missing {missing[0]}")
        _clear_outputs(paths)
        return BuildResult((), reason)

    try:
        comparison = load_comparison(input_root, target)
    except MetricsError as error:
        reason = str(error)
        if required:
            raise
        _clear_outputs(paths)
        return BuildResult((), reason)

    rendered = tuple(
        render_comparison(comparison, target, precision)
        for precision in PRECISIONS
    )
    output.mkdir(parents=True, exist_ok=True)
    partial_paths: list[Path] = []
    for content, path in zip(rendered, paths):
        partial_path = path.with_suffix(".partial" + path.suffix)
        partial_paths.append(partial_path)
        try:
            partial_path.write_text(
                content,
                encoding="utf-8",
                newline="\n",
            )
            partial_path.replace(path)
        except OSError as error:
            _clear_outputs((*paths, *partial_paths))
            raise MetricsError(f"cannot write {path}: {error}") from error
    return BuildResult(paths)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    metrics_dir = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=metrics_dir / "data")
    parser.add_argument(
        "--output",
        type=Path,
        default=metrics_dir / "generated" / "profile_comparison",
    )
    parser.add_argument("--target", type=build_tables._target, required=True)
    parser.add_argument("--required", action="store_true", help=argparse.SUPPRESS)
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        result = build(
            args.input.resolve(),
            args.output.resolve(),
            args.target,
            required=args.required,
        )
    except (MetricsError, OSError, ValueError) as error:
        print(f"profile comparison failed: {error}", file=sys.stderr)
        return 1
    if result.skipped is not None:
        print(f"profile comparison skipped: {result.skipped}", flush=True)
    else:
        for path in result.outputs:
            print(path, flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
