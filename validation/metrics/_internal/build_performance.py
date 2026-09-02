#!/usr/bin/env python3
"""Build the cross-target FLTX performance comparison SVG."""

from __future__ import annotations

import argparse
import base64
import csv
import html
import json
import math
import re
import sys
from dataclasses import dataclass
from pathlib import Path

from build_tables import (
    GROUP_LABELS,
    GROUP_ORDER,
    OPERATION_ORDER,
    REPORT_EXCLUDED_OPERATIONS,
    Target,
    _OPERATIONS_BY_GROUP,
    _ratio_color,
    _report_group,
    architecture_label,
    compiler_label_lines,
    platform_label,
)
from run_metrics import (
    CANONICAL_FIELDS,
    CONSUMER_MODES,
    MINIMUM_CREDIBLE_BENCHMARK_NS,
    SCHEMA_VERSION,
    MetricsError,
    _safe_component,
    consumer_mode_suffix,
)


PRECISIONS = ("dd", "qd")
PRECISION_LABELS = {"dd": "bl::fdd", "qd": "bl::fqd"}
CANONICAL_FILE_RE = re.compile(
    r"(?P<compiler>.+)_(?P<precision>dd|qd)"
    r"(?P<consumer_suffix>_fastmath)?\.csv$",
    re.IGNORECASE,
)

PLATFORM_ORDER = {
    "windows": 0,
    "linux": 1,
    "macos": 2,
    "webassembly": 3,
}
COMPILER_ORDER = {
    "msvc": 0,
    "clangcl": 1,
    "mingw": 2,
    "gcc": 3,
    "clang": 4,
    "appleclang": 5,
    "nodejs": 5,
    "node": 5,
    "chrome": 6,
    "browser": 6,
    "emscripten": 7,
}
ARCHITECTURE_ORDER = {"x86_64": 0, "arm64": 1, "wasm32": 2}

PAGE = "#0f1115"
FUNCTION_HEADER = "#2b2b2b"
HEADER = "#244b70"
PLATFORM_HEADER = "#293f57"
LEAF_HEADER = "#313d4b"
GROUP = "#4e555f"
GRID = "#000000"
INK = "#f1f2f4"
MUTED = "#aeb4bf"
WHITE = "#ffffff"
BODY_TEXT = "#e7e9ee"
BODY_ROW_FILLS = ("#191d23", "#16191f")
CELL_PADDING = 8

COMPACT_FONT_ROOT = (
    Path(__file__).resolve().parents[1] /
    "assets" / "fonts" / "ubuntu-mono"
)
COMPACT_FONTS = {
    400: COMPACT_FONT_ROOT / "UbuntuMono-Regular.woff2",
    700: COMPACT_FONT_ROOT / "UbuntuMono-Bold.woff2",
}

TABLE_TITLE = "fltx performance"
TABLE_DESCRIPTION = (
    "Columns identify the target platform and build toolchain. FLTX time per "
    "iteration; the second line shows FLTX speed versus the fastest available "
    "competitor for each target."
)


def _font_css(layout: str) -> str:
    if layout != "compact":
        return (
            "<style>text { font-family: Consolas, 'Courier New', "
            "monospace; }</style>"
        )

    faces = []
    for weight, path in COMPACT_FONTS.items():
        encoded = base64.b64encode(path.read_bytes()).decode("ascii")
        faces.append(
            "@font-face {"
            "font-family:'Ubuntu Mono';"
            f"font-weight:{weight};"
            "font-style:normal;"
            f"src:url(data:font/woff2;base64,{encoded}) format('woff2');"
            "}"
        )
    return (
        "<style>" + "".join(faces) +
        "text { font-family:'Ubuntu Mono', Consolas, monospace; }</style>"
    )


@dataclass(frozen=True)
class ColumnKey:
    precision: str
    target: Target


@dataclass
class PerformanceDataset:
    columns: tuple[ColumnKey, ...]
    rows: dict[tuple[ColumnKey, str, str, str], dict[str, str]]
    implementations: dict[tuple[ColumnKey, str, str], tuple[str, ...]]
    run_ids: dict[ColumnKey, str]
    sources: dict[ColumnKey, Path]
    consumer_mode: str = "strict"


@dataclass(frozen=True)
class ResultCell:
    fltx: dict[str, str]
    competitor: dict[str, str] | None
    fltx_ns: float
    competitor_ns: float | None
    fltx_speed_ratio: float | None


_CANONICAL_GROUP_BY_OPERATION: dict[str, str] = {}
for _group, _operations in _OPERATIONS_BY_GROUP.items():
    for _operation in _operations:
        if _operation in _CANONICAL_GROUP_BY_OPERATION:
            raise RuntimeError(f"duplicate report operation {_operation!r}")
        _CANONICAL_GROUP_BY_OPERATION[_operation] = _group


def _normalized_key(value: str) -> str:
    return re.sub(r"[^a-z0-9]", "", value.casefold())


def _column_sort_key(column: ColumnKey) -> tuple[int, int, str, int, str, int, str]:
    platform_key = column.target.platform.casefold()
    compiler_key = _normalized_key(column.target.compiler)
    return (
        PRECISIONS.index(column.precision),
        PLATFORM_ORDER.get(platform_key, len(PLATFORM_ORDER)),
        platform_label(column.target.platform).casefold(),
        ARCHITECTURE_ORDER.get(
            column.target.architecture.casefold(), len(ARCHITECTURE_ORDER)
        ),
        column.target.architecture.casefold(),
        COMPILER_ORDER.get(compiler_key, len(COMPILER_ORDER)),
        column.target.compiler.casefold(),
    )


def _canonical_group(group: str, operation: str) -> str:
    return _CANONICAL_GROUP_BY_OPERATION.get(operation, group)


def _read_canonical(path: Path, column: ColumnKey) -> list[dict[str, str]]:
    try:
        with path.open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream)
            if tuple(reader.fieldnames or ()) != CANONICAL_FIELDS:
                raise MetricsError(f"{path}: incompatible schema/header")
            rows = list(reader)
    except OSError as error:
        raise MetricsError(f"cannot read {path}: {error}") from error

    if not rows:
        raise MetricsError(f"{path}: contains no rows")
    if any(None in row or any(value is None for value in row.values()) for row in rows):
        raise MetricsError(f"{path}: malformed row width")

    seen: set[tuple[str, str, str]] = set()
    run_ids: set[str] = set()
    normalized: list[dict[str, str]] = []
    for source_row in rows:
        row = dict(source_row)
        if row["precision"] != column.precision:
            raise MetricsError(
                f"{path}: row precision {row['precision']!r} does not match "
                f"{column.precision!r}"
            )
        group = _canonical_group(row["group"], row["operation"])
        operation = row["operation"]
        implementation = row["implementation"]
        if not group or not operation or not implementation:
            raise MetricsError(f"{path}: empty benchmark identity")
        row["group"] = group
        key = (group, operation, implementation)
        if key in seen:
            raise MetricsError(f"{path}: duplicate canonical row {key}")
        seen.add(key)

        run_id = row["run_id"]
        if not run_id:
            raise MetricsError(f"{path}: empty run_id")
        run_ids.add(run_id)
        if row["ns_iter"]:
            _positive_float(row["ns_iter"], path, key)
        normalized.append(row)

    if len(run_ids) != 1:
        raise MetricsError(f"{path}: rows contain multiple run IDs")
    if not any(
        row["implementation"] == "fltx" and row["ns_iter"]
        for row in normalized
    ):
        raise MetricsError(f"{path}: contains no FLTX benchmark timings")
    return normalized


def _positive_float(
    value: str,
    source: Path,
    identity: tuple[str, str, str],
) -> float:
    try:
        result = float(value)
    except ValueError as error:
        raise MetricsError(
            f"{source}: invalid benchmark timing {value!r} for {identity}"
        ) from error
    if not math.isfinite(result) or result <= 0.0:
        raise MetricsError(
            f"{source}: benchmark timing must be positive and finite for {identity}"
        )
    if result < MINIMUM_CREDIBLE_BENCHMARK_NS:
        raise MetricsError(
            f"{source}: implausibly small benchmark timing for {identity}; "
            "the workload may have been optimized away"
        )
    return result


def discover(
    input_root: Path,
    consumer_mode: str = "strict",
) -> PerformanceDataset:
    """Load every canonical platform/architecture/compiler CSV independently."""

    if not input_root.is_dir():
        raise MetricsError(f"metrics input directory does not exist: {input_root}")

    rows: dict[tuple[ColumnKey, str, str, str], dict[str, str]] = {}
    implementation_lists: dict[tuple[ColumnKey, str, str], list[str]] = {}
    run_ids: dict[ColumnKey, str] = {}
    sources: dict[ColumnKey, Path] = {}

    for platform_dir in sorted(input_root.iterdir(), key=lambda path: path.name.casefold()):
        if not platform_dir.is_dir():
            continue
        for architecture_dir in sorted(
            platform_dir.iterdir(), key=lambda path: path.name.casefold()
        ):
            if not architecture_dir.is_dir():
                continue
            for path in sorted(
                architecture_dir.iterdir(), key=lambda item: item.name.casefold()
            ):
                if not path.is_file():
                    continue
                match = CANONICAL_FILE_RE.fullmatch(path.name)
                if match is None:
                    continue
                observed_mode = (
                    "fastmath" if match.group("consumer_suffix") else "strict"
                )
                if observed_mode != consumer_mode:
                    continue
                try:
                    target = Target(
                        _safe_component(platform_dir.name, "platform"),
                        _safe_component(architecture_dir.name, "architecture"),
                        _safe_component(match.group("compiler"), "compiler"),
                    )
                except MetricsError as error:
                    raise MetricsError(f"{path}: {error}") from error
                column = ColumnKey(match.group("precision").casefold(), target)
                if column in sources:
                    raise MetricsError(
                        f"duplicate performance source for {column.precision} "
                        f"{column.target.label}: {sources[column]} and {path}"
                    )

                source_rows = _read_canonical(path, column)
                sources[column] = path
                run_ids[column] = source_rows[0]["run_id"]
                for row in source_rows:
                    group = row["group"]
                    operation = row["operation"]
                    implementation = row["implementation"]
                    rows[(column, group, operation, implementation)] = row
                    key = (column, group, operation)
                    values = implementation_lists.setdefault(key, [])
                    if implementation not in values:
                        values.append(implementation)

    columns = tuple(sorted(sources, key=_column_sort_key))
    implementations = {
        key: tuple(values)
        for key, values in implementation_lists.items()
    }
    return PerformanceDataset(
        columns,
        rows,
        implementations,
        run_ids,
        sources,
        consumer_mode,
    )


def operations(dataset: PerformanceDataset) -> list[tuple[str, str]]:
    pairs = {
        (group, operation)
        for (column, group, operation, implementation), row in dataset.rows.items()
        if (
            implementation == "fltx"
            and row["ns_iter"]
            and (group, operation) not in REPORT_EXCLUDED_OPERATIONS
        )
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


def _result_cell(
    dataset: PerformanceDataset,
    column: ColumnKey,
    group: str,
    operation: str,
) -> ResultCell | None:
    fltx = dataset.rows.get((column, group, operation, "fltx"))
    if fltx is None or not fltx["ns_iter"]:
        return None
    fltx_ns = float(fltx["ns_iter"])

    candidates: list[tuple[float, str, str, dict[str, str]]] = []
    for implementation in dataset.implementations.get(
        (column, group, operation), (),
    ):
        if implementation == "fltx":
            continue
        row = dataset.rows.get((column, group, operation, implementation))
        if row is None or not row["ns_iter"]:
            continue
        competitor_ns = float(row["ns_iter"])
        candidates.append((
            competitor_ns,
            row["implementation_short"],
            implementation,
            row,
        ))
    if not candidates:
        return ResultCell(fltx, None, fltx_ns, None, None)

    competitor_ns, _, _, competitor = min(candidates)
    return ResultCell(
        fltx,
        competitor,
        fltx_ns,
        competitor_ns,
        competitor_ns / fltx_ns,
    )


def _format_ns(value: float, layout: str) -> str:
    if layout == "compact":
        if value >= 100.0:
            return f"{value:,.0f}ns"
        if value >= 1.0:
            return f"{value:.1f}ns"
        return f"{value:.2f}ns"
    if value >= 100.0:
        return f"{value:,.0f}ns"
    if value >= 10.0:
        return f"{value:.1f}ns"
    if value >= 1.0:
        return f"{value:.2f}ns"
    return f"{value:.3f}ns"


def _format_ratio(value: float, layout: str) -> str:
    return f"{value:.1f}×" if layout == "compact" else f"{value:.2f}×"


def _result_lines(cell: ResultCell, layout: str) -> tuple[str, ...]:
    lines = [_format_ns(cell.fltx_ns, layout)]
    if cell.competitor is not None and cell.fltx_speed_ratio is not None:
        lines.append(
            f"{_format_ratio(cell.fltx_speed_ratio, layout)} "
            f"{cell.competitor['implementation_short']}"
        )
    return tuple(lines)


def _tooltip(
    column: ColumnKey,
    group: str,
    operation: str,
    cell: ResultCell,
) -> str:
    lines = [
        f"{column.target.label}; {PRECISION_LABELS[column.precision]}; "
        f"{group} / {operation}",
        f"{cell.fltx['implementation_label']} ({cell.fltx['api']}): "
        f"{cell.fltx_ns:g} ns",
    ]
    if (
        cell.competitor is not None
        and cell.competitor_ns is not None
        and cell.fltx_speed_ratio is not None
    ):
        lines.append(
            f"Fastest competitor: {cell.competitor['implementation_label']} "
            f"({cell.competitor['api']}): {cell.competitor_ns:g} ns; "
            f"FLTX speed vs fastest competitor: "
            f"{cell.fltx_speed_ratio:.6g}×"
        )
    else:
        lines.append("No competitor benchmark is available.")
    return "\n".join(lines)


def _svg_text(
    x: float,
    y: float,
    value: str,
    size: int,
    color: str,
    *,
    weight: int = 400,
    anchor: str = "start",
    css_class: str = "",
) -> str:
    class_attribute = (
        f' class="{html.escape(css_class, quote=True)}"' if css_class else ""
    )
    return (
        f'<text x="{x:g}" y="{y:g}" fill="{color}" font-size="{size}" '
        f'font-weight="{weight}" text-anchor="{anchor}"{class_attribute}>'
        f"{html.escape(value)}</text>"
    )


def _svg_rect(
    x: float,
    y: float,
    width: float,
    height: float,
    fill: str,
    *,
    stroke: str = GRID,
    css_class: str = "",
) -> str:
    class_attribute = (
        f' class="{html.escape(css_class, quote=True)}"' if css_class else ""
    )
    return (
        f'<rect x="{x:g}" y="{y:g}" width="{width:g}" height="{height:g}" '
        f'fill="{fill}" stroke="{stroke}" stroke-width="1"{class_attribute}/>'
    )


def _precision_spans(
    columns: tuple[ColumnKey, ...],
) -> list[tuple[int, int]]:
    spans: list[tuple[int, int]] = []
    index = 0
    while index < len(columns):
        precision = columns[index].precision
        count = 1
        while (
            index + count < len(columns)
            and columns[index + count].precision == precision
        ):
            count += 1
        spans.append((index, count))
        index += count
    return spans


def _reference_legend(
    dataset: PerformanceDataset,
    operation_rows: list[tuple[str, str]],
) -> list[str]:
    lines: list[str] = []
    for precision in PRECISIONS:
        references: dict[str, str] = {}
        for column in dataset.columns:
            if column.precision != precision:
                continue
            for group, operation in operation_rows:
                cell = _result_cell(dataset, column, group, operation)
                if cell is None or cell.competitor is None:
                    continue
                short = cell.competitor["implementation_short"]
                label = cell.competitor["implementation_label"]
                existing = references.get(short)
                if existing is not None and existing != label:
                    raise MetricsError(
                        f"ambiguous implementation short name {short!r}"
                    )
                references[short] = label
        if references:
            if lines:
                lines.append("")
            lines.extend(
                f"{short} = {references[short]}"
                for short in sorted(references, key=str.casefold)
            )
    return lines


def _operation_width(
    operation_rows: list[tuple[str, str]],
    layout: str,
) -> int:
    longest = max([len("Function")] + [len(operation) for _, operation in operation_rows])
    if layout == "compact":
        return max(96, math.ceil(longest * 6.0) + 16)
    natural = max(210, min(720, longest * 8 + 34))
    return round(natural * 0.75)


def _column_width(
    dataset: PerformanceDataset,
    column: ColumnKey,
    operation_rows: list[tuple[str, str]],
    layout: str,
) -> int:
    if layout == "full":
        return 116
    values = [
        *compiler_label_lines(column.target.compiler),
        platform_label(column.target.platform),
        architecture_label(column.target.architecture),
        PRECISION_LABELS[column.precision],
    ]
    for group, operation in operation_rows:
        cell = _result_cell(dataset, column, group, operation)
        if cell is not None:
            values.extend(_result_lines(cell, layout))
    return max(78, math.ceil(max(map(len, values), default=0) * 6.0) + 12)


def _grouped_operations(
    operation_rows: list[tuple[str, str]],
) -> list[tuple[str, list[tuple[str, str]]]]:
    grouped: list[tuple[str, list[tuple[str, str]]]] = []
    for group, operation in operation_rows:
        report_group = _report_group(group)
        if not grouped or grouped[-1][0] != report_group:
            grouped.append((report_group, []))
        grouped[-1][1].append((group, operation))
    return grouped


def render(dataset: PerformanceDataset, layout: str = "full") -> str:
    if layout not in {"full", "compact"}:
        raise ValueError(f"unsupported performance layout {layout!r}")

    compact = layout == "compact"
    table_title = (
        "fltx consumer fast-math performance"
        if dataset.consumer_mode == "fastmath"
        else TABLE_TITLE
    )
    table_description = (
        TABLE_DESCRIPTION[:-1] + " with consumer fast-math enabled."
        if dataset.consumer_mode == "fastmath"
        else TABLE_DESCRIPTION
    )
    margin = 0 if compact else 18
    type_gap = 6 if compact else 16
    header_height = 28 if compact else 34
    group_height = 20 if compact else 28
    row_height = 30 if compact else 40
    operation_rows = operations(dataset)
    grouped = _grouped_operations(operation_rows)
    legend = [] if compact else _reference_legend(dataset, operation_rows)
    legend_height = 0
    if legend:
        legend_height = 8 + sum(8 if not line else 15 for line in legend)
    title_height = 0 if compact else 62 + legend_height

    operation_width = _operation_width(operation_rows, layout)
    column_widths = tuple(
        _column_width(dataset, column, operation_rows, layout)
        for column in dataset.columns
    )
    boundaries = {
        index
        for index in range(1, len(dataset.columns))
        if dataset.columns[index].precision != dataset.columns[index - 1].precision
    }
    column_x: list[int] = []
    x = margin + operation_width
    for index, width in enumerate(column_widths):
        if index in boundaries:
            x += type_gap
        column_x.append(x)
        x += width
    table_width = x - margin
    width = margin * 2 + table_width
    if not compact:
        width = max(
            width,
            margin * 2 + math.ceil(len(table_description) * 8.3),
            margin * 2 + max(
                [0] + [math.ceil(len(line) * 7.6) for line in legend]
            ),
        )
    height = (
        margin * 2
        + title_height
        + header_height * 4
        + group_height * len(grouped)
        + row_height * len(operation_rows)
    )
    if not dataset.columns:
        width = max(width, 640 if not compact else 320)
        height = max(height, margin * 2 + title_height + 56)

    parts = [
        (
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" '
            f'height="{height}" viewBox="0 0 {width} {height}" role="img" '
            'aria-labelledby="title desc">'
        ),
        f'<title id="title">{html.escape(table_title)}</title>',
        f'<desc id="desc">{html.escape(table_description)}</desc>',
        _font_css(layout),
        f'<rect width="{width}" height="{height}" fill="{PAGE}"/>',
    ]
    if not compact:
        parts.extend([
            _svg_text(margin, margin + 26, table_title, 26, INK, weight=700),
            _svg_text(
                margin,
                margin + 50,
                table_description,
                15,
                MUTED,
            ),
        ])
        legend_y = margin + 76
        for line in legend:
            if not line:
                legend_y += 8
                continue
            short, _, label = line.partition(" = ")
            short_width = math.ceil(len(short) * 8.2)
            parts.append(
                _svg_text(margin, legend_y, short, 13, INK, weight=700)
            )
            parts.append(
                _svg_text(
                    margin + short_width + 8,
                    legend_y,
                    "=",
                    13,
                    INK,
                    weight=650,
                )
            )
            parts.append(
                _svg_text(
                    margin + short_width + 22,
                    legend_y,
                    label,
                    13,
                    MUTED,
                    weight=600,
                )
            )
            legend_y += 15

    y = margin + title_height
    if not dataset.columns:
        parts.append(
            _svg_text(
                margin,
                y + 32,
                "No canonical performance metrics were found.",
                16,
                MUTED,
            )
        )
        parts.append("</svg>")
        return "\n".join(parts) + "\n"

    parts.append(
        _svg_rect(
            margin,
            y,
            operation_width,
            header_height * 4,
            FUNCTION_HEADER,
            css_class="function-header-cell",
        )
    )
    parts.append(
        _svg_text(
            margin + CELL_PADDING,
            y + header_height * 2 + 5,
            "Function",
            12 if compact else 14,
            WHITE,
            weight=700,
        )
    )

    for start, count in _precision_spans(dataset.columns):
        span_width = sum(column_widths[start:start + count])
        parts.append(
            _svg_rect(
                column_x[start],
                y,
                span_width,
                header_height,
                HEADER,
                css_class="precision-header-cell",
            )
        )
        parts.append(
            _svg_text(
                column_x[start] + span_width / 2,
                y + header_height // 2 + 5,
                PRECISION_LABELS[dataset.columns[start].precision],
                12 if compact else 14,
                WHITE,
                weight=700,
                anchor="middle",
                css_class="precision-header",
            )
        )

    platform_y = y + header_height
    index = 0
    while index < len(dataset.columns):
        column = dataset.columns[index]
        count = 1
        while (
            index + count < len(dataset.columns)
            and dataset.columns[index + count].precision == column.precision
            and dataset.columns[index + count].target.platform
            == column.target.platform
        ):
            count += 1
        span_width = sum(column_widths[index:index + count])
        parts.append(
            _svg_rect(
                column_x[index],
                platform_y,
                span_width,
                header_height,
                PLATFORM_HEADER,
                css_class="platform-header-cell",
            )
        )
        parts.append(
            _svg_text(
                column_x[index] + span_width / 2,
                platform_y + header_height // 2 + 5,
                platform_label(column.target.platform),
                12 if compact else 14,
                WHITE,
                weight=700,
                anchor="middle",
                css_class="platform-header",
            )
        )
        index += count

    architecture_y = platform_y + header_height
    index = 0
    while index < len(dataset.columns):
        column = dataset.columns[index]
        count = 1
        while (
            index + count < len(dataset.columns)
            and dataset.columns[index + count].precision == column.precision
            and dataset.columns[index + count].target.platform
            == column.target.platform
            and dataset.columns[index + count].target.architecture
            == column.target.architecture
        ):
            count += 1
        span_width = sum(column_widths[index:index + count])
        parts.append(
            _svg_rect(
                column_x[index],
                architecture_y,
                span_width,
                header_height,
                LEAF_HEADER,
                css_class="architecture-header-cell",
            )
        )
        parts.append(
            _svg_text(
                column_x[index] + span_width / 2,
                architecture_y + header_height // 2 + 5,
                architecture_label(column.target.architecture),
                12 if compact else 14,
                WHITE,
                weight=700,
                anchor="middle",
                css_class="architecture-header",
            )
        )
        index += count

    toolchain_y = architecture_y + header_height
    for index, column in enumerate(dataset.columns):
        parts.append(
            _svg_rect(
                column_x[index],
                toolchain_y,
                column_widths[index],
                header_height,
                LEAF_HEADER,
                css_class="toolchain-header-cell",
            )
        )
        labels = compiler_label_lines(column.target.compiler)
        center_x = column_x[index] + column_widths[index] / 2
        accessible_label = (
            labels[0] if len(labels) == 1 else f"{labels[0]} ({labels[1]})"
        )
        parts.append(
            f'<g class="toolchain-header" aria-label="'
            f'{html.escape(accessible_label, quote=True)}">'
        )
        if len(labels) == 1:
            parts.append(
                _svg_text(
                    center_x,
                    toolchain_y + header_height // 2 + 5,
                    labels[0],
                    12 if compact else 14,
                    WHITE,
                    weight=700,
                    anchor="middle",
                    css_class="toolchain-header-primary",
                )
            )
        else:
            parts.extend([
                _svg_text(
                    center_x,
                    toolchain_y + (11 if compact else 14),
                    labels[0],
                    11 if compact else 13,
                    WHITE,
                    weight=700,
                    anchor="middle",
                    css_class="toolchain-header-primary",
                ),
                _svg_text(
                    center_x,
                    toolchain_y + (23 if compact else 29),
                    labels[1],
                    10 if compact else 11,
                    MUTED,
                    weight=700,
                    anchor="middle",
                    css_class="toolchain-header-environment",
                ),
            ])
        parts.append("</g>")

    y += header_height * 4
    row_index = 0
    for report_group, rows_in_group in grouped:
        spans = _precision_spans(dataset.columns)
        for span_index, (start, count) in enumerate(spans):
            span_x = margin if span_index == 0 else column_x[start]
            span_width = sum(column_widths[start:start + count])
            if span_index == 0:
                span_width += operation_width
            parts.append(
                _svg_rect(span_x, y, span_width, group_height, GROUP)
            )
        parts.append(
            _svg_text(
                margin + CELL_PADDING,
                y + group_height // 2 + 5,
                GROUP_LABELS.get(report_group, report_group),
                11 if compact else 13,
                WHITE,
                weight=700,
                css_class="group-label",
            )
        )
        y += group_height

        for group, operation in rows_in_group:
            body_fill = BODY_ROW_FILLS[row_index % len(BODY_ROW_FILLS)]
            parts.append(
                _svg_rect(
                    margin,
                    y,
                    operation_width,
                    row_height,
                    body_fill,
                )
            )
            parts.append(
                _svg_text(
                    margin + CELL_PADDING,
                    y + row_height // 2 + 5,
                    operation,
                    11 if compact else 14,
                    BODY_TEXT,
                    weight=400,
                    css_class="operation-label",
                )
            )
            for index, column in enumerate(dataset.columns):
                cell = _result_cell(dataset, column, group, operation)
                if cell is None:
                    parts.append(
                        _svg_rect(
                            column_x[index],
                            y,
                            column_widths[index],
                            row_height,
                            body_fill,
                        )
                    )
                    continue

                parts.append("<g>")
                parts.append(
                    f"<title>{html.escape(_tooltip(column, group, operation, cell))}</title>"
                )
                parts.append(
                    _svg_rect(
                        column_x[index],
                        y,
                        column_widths[index],
                        row_height,
                        body_fill,
                    )
                )
                lines = _result_lines(cell, layout)
                if len(lines) == 1:
                    baselines = (y + row_height // 2 + 5,)
                else:
                    first = y + (12 if compact else 17)
                    baselines = (first, first + (13 if compact else 16))
                for line_index, (line, baseline) in enumerate(
                    zip(lines, baselines)
                ):
                    foreground = (
                        _ratio_color(cell.fltx_speed_ratio)
                        if line_index > 0
                        and cell.fltx_speed_ratio is not None
                        else BODY_TEXT
                    )
                    parts.append(
                        _svg_text(
                            column_x[index] + column_widths[index] / 2,
                            baseline,
                            line,
                            11 if compact else (14 if line == lines[0] else 12),
                            foreground,
                            weight=400,
                            anchor="middle",
                            css_class="result-line",
                        )
                    )
                parts.append("</g>")
            y += row_height
            row_index += 1

    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def _write_atomic(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    partial = path.with_suffix(".partial" + path.suffix)
    partial.write_text(text, encoding="utf-8", newline="\n")
    partial.replace(path)


def build(
    input_root: Path,
    output: Path,
    layout: str = "full",
    consumer_mode: str = "strict",
) -> tuple[Path, Path]:
    dataset = discover(input_root, consumer_mode)
    suffix = consumer_mode_suffix(consumer_mode)
    if layout == "compact":
        suffix += "_compact"
    svg_path = output / f"performance_table{suffix}.svg"
    metadata_path = output / f"performance_run{suffix}.json"
    _write_atomic(svg_path, render(dataset, layout))
    metadata = {
        "schema_version": SCHEMA_VERSION,
        "consumer_mode": consumer_mode,
        "layout": layout,
        "output": svg_path.name,
        "sources": [
            {
                "platform": column.target.platform,
                "architecture": column.target.architecture,
                "compiler": column.target.compiler,
                "precision": column.precision,
                "run_id": dataset.run_ids[column],
                "file": dataset.sources[column]
                .relative_to(input_root)
                .as_posix(),
            }
            for column in dataset.columns
        ],
    }
    _write_atomic(
        metadata_path,
        json.dumps(metadata, indent=2, sort_keys=True) + "\n",
    )
    return svg_path, metadata_path


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input",
        type=Path,
        default=Path(__file__).parents[1] / "data",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(__file__).parents[1] / "generated",
    )
    parser.add_argument(
        "--layout",
        choices=("full", "compact"),
        default="full",
    )
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
            args.output,
            args.layout,
            args.consumer_mode,
        )
    except (MetricsError, OSError, ValueError) as error:
        print(f"performance table build failed: {error}", file=sys.stderr)
        return 1
    for output in outputs:
        print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
