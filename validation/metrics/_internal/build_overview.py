#!/usr/bin/env python3
"""Build compact, original-style metrics overview tables for one target."""

from __future__ import annotations

import argparse
import base64
import colorsys
import html
import math
import sys
from functools import partial
from pathlib import Path

from build_tables import (
    GROUP_LABELS,
    OPERATION_ORDER,
    GROUP_ORDER,
    REPORT_EXCLUDED_OPERATIONS,
    Dataset,
    Target,
    _ratio_color,
    _report_group,
    _target,
    load,
)
from run_metrics import CONSUMER_MODES, MetricsError, consumer_mode_suffix
from manifest import IMPLEMENTATIONS


PRECISIONS = ("dd", "qd")
PRECISION_TYPE_NAMES = {"dd": "fdd", "qd": "fqd"}
SUPPORT = {
    "Both": "#4ade80",
    "Inf": "#fbbf24",
    "NaN": "#fbbf24",
    "No": "#f87171",
    "-": "#a5abb5",
}
PRIMARY_HEADER_LABELS = {
    "ddreal": "qdpp (dd_real)",
    "qdreal": "qdpp (qd_real)",
    "cppdd": "boost (cpp_double_double)",
    "mpfr64": "boost (mpfr_float_backend<64>)",
    "tlquad": "TLFloat (Quad)",
    "tloct": "TLFloat (Octuple)",
}
BODY_LIGHTNESS = (29 * 1.1) / 255


def _normalized_body_tint(color: str) -> str:
    channels = tuple(
        int(color[index:index + 2], 16) / 255
        for index in (1, 3, 5)
    )
    hue, _, saturation = colorsys.rgb_to_hls(*channels)
    normalized = colorsys.hls_to_rgb(hue, BODY_LIGHTNESS, saturation)
    return "#" + "".join(f"{round(channel * 255):02x}" for channel in normalized)


def _brighter_tint(color: str, factor: float = 1.1) -> str:
    channels = tuple(
        int(color[index:index + 2], 16) / 255
        for index in (1, 3, 5)
    )
    hue, lightness, saturation = colorsys.rgb_to_hls(*channels)
    adjusted = colorsys.hls_to_rgb(
        hue,
        min(1.0, lightness * factor),
        saturation,
    )
    return "#" + "".join(f"{round(channel * 255):02x}" for channel in adjusted)


_IMPLEMENTATION_TINTS = (
    ("#40354f", "#1d1c1f"),
    ("#313d4b", "#1c1d1f"),
    ("#3e4232", "#1c1c1b"),
    ("#493637", "#1e1c1d"),
    ("#314641", "#1b1d1d"),
    ("#42364a", "#1e1c1f"),
)
IMPLEMENTATION_TINTS = tuple(
    (header, _normalized_body_tint(body))
    for header, body in _IMPLEMENTATION_TINTS
)
FLTX_TINT = ("#244b70", _normalized_body_tint("#202428"))
OPERATION_ROW_FILLS = tuple(
    _brighter_tint(color)
    for color in ("#171a20", "#14171c")
)

TABLE_GAP = 8
COMPACT_TABLE_GAP = 3
MONOSPACE_CHARACTER_WIDTH = 6
COMPACT_FONT_ADVANCE_EM = 0.5
COMPACT_FONT_SIZE_INCREMENT = -1
COMPACT_EXACT_STROKE_WIDTH = 1.1
COMPACT_EXACT_SCALE = 0.5625
COMPACT_REFERENCE_MATCH_FONT_SIZE = 14
COMPACT_ACCURACY_EXTRA_WIDTH = 2
COMPACT_BENCHMARK_UNIT_GAP = 3
COMPACT_OPERATION_TEXT_SHIFT = 2
COMPACT_SPECIAL_WIDTH_REDUCTION = 4
CELL_PADDING = 8
COMPACT_CELL_PADDING = 4
GRID_REGULAR = "#000000"
GRID_STRONG = "#000000"
GROUP_ROW_FILL = "#4e555f"
GROUP_ROW_TEXT = "#ffffff"
EXACT_TEXT = "#4ade80"
COMPACT_EXACT_TEXT = "#6ee7a0"
BENCHMARK_SEPARATOR_TEXT = "#cfd3da"
COMPACT_BENCHMARK_UNIT_TEXT = "#b8bdc5"
COLUMN_SPECS = (
    ("mean", ("mean",), 44),
    ("worst", ("worst",), 44),
    ("pass", ("pass",), 40),
    ("benchmark", ("time · speed vs FLTX",), 40),
    ("special", (), 40),
    ("signed_zero", (), 24),
)
SUBNORMAL_COLUMN_SPEC = ("subnormal", (), 52)
SUBNORMAL_DOMAIN = "subnormal"
MARGIN = 12
COMPACT_MARGIN = 0
TITLE_HEIGHT = 54
HEADER_HEIGHTS = (24, 28, 32)
COMPACT_HEADER_HEIGHTS = (24, 34, 32)
ROW_HEIGHT = 22
GROUP_ROW_HEIGHT = 18
FOOTER_HEIGHT = 34
FONT_SCALE = 1.2
COMPACT_FONT_ROOT = (
    Path(__file__).resolve().parents[1] /
    "assets" / "fonts" / "ubuntu-mono"
)
COMPACT_FONTS = {
    400: COMPACT_FONT_ROOT / "UbuntuMono-Regular.woff2",
    700: COMPACT_FONT_ROOT / "UbuntuMono-Bold.woff2",
}


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
    faces.append(
        "text { font-family:'Ubuntu Mono', Consolas, monospace; "
        "text-rendering:optimizeLegibility; }"
    )
    return "<style>" + "".join(faces) + "</style>"


def _rendered_font_size(
    font_size: int,
    font_size_increment: int = 0,
    snap_to_pixels: bool = False,
) -> float:
    rendered_size = font_size * FONT_SCALE + font_size_increment
    return round(rendered_size) if snap_to_pixels else rendered_size


def _estimated_character_width(layout: str, font_size: int) -> float:
    if layout == "compact":
        rendered_size = _rendered_font_size(
            font_size,
            COMPACT_FONT_SIZE_INCREMENT,
            snap_to_pixels=True,
        )
        return rendered_size * COMPACT_FONT_ADVANCE_EM
    return MONOSPACE_CHARACTER_WIDTH * FONT_SCALE


def _estimated_text_width(value: str, layout: str, font_size: int) -> float:
    character_width = _estimated_character_width(layout, font_size)
    width = len(value) * character_width
    if layout == "compact" and value.endswith(" ns"):
        width += COMPACT_BENCHMARK_UNIT_GAP - character_width
    return width


def _fmt_bits(value: str) -> str:
    if not value:
        return "-"
    number = float(value)
    if math.isinf(number):
        return "exact"
    return f"{number:.1f}"


def _fmt_bits_compact(value: str) -> str:
    if not value:
        return "-"
    number = float(value)
    if math.isinf(number):
        return "="
    return f"{number:.0f}"


def _fmt_ns(value: str) -> str:
    if not value:
        return "-"
    number = float(value)
    if number >= 1000:
        return f"{number:,.0f}"
    if number >= 100:
        return f"{number:.1f}"
    if number >= 1:
        return f"{number:.2f}"
    return f"{number:.3f}"


def _fmt_ns_compact(value: str) -> str:
    """Use one fewer decimal place for unitless compact FLTX timings."""

    number = float(value)
    if number >= 1000:
        return f"{number:.0f}"
    if number >= 100:
        return f"{number:.0f}"
    if number >= 1:
        return f"{number:.1f}"
    return f"{number:.2f}"


def _fmt_ratio(value: str | float) -> str:
    return "-" if not value else f"{float(value):.2f}×"


def _speed_vs_fltx(
    row: dict[str, str],
    implementation: str,
) -> float | None:
    if implementation == "fltx" or not row["speed_ratio"]:
        return None
    time_ratio = float(row["speed_ratio"])
    if time_ratio <= 0:
        raise MetricsError("benchmark time ratio must be positive")
    return 1.0 / time_ratio


def _domain_style(
    row: dict[str, str] | None,
    body_fill: str = "#33363d",
) -> tuple[str, str, str]:
    if row is None or not row["domains_total"]:
        return "-", body_fill, "#a5abb5"
    passed = int(row["domains_passed"])
    total = int(row["domains_total"])
    margin = float(row["min_margin_bits"])
    text = f"{passed}/{total}"
    if passed != total:
        return text, body_fill, "#f87171"
    if margin < 4:
        return text, body_fill, "#fbbf24"
    return text, body_fill, "#4ade80"


def _special_style(
    row: dict[str, str] | None,
    body_fill: str = "#33363d",
) -> tuple[str, str, str]:
    category = "-" if row is None else row.get("special_support", "-")
    if category not in SUPPORT:
        raise MetricsError(f"malformed special_support category {category!r}")
    return category, body_fill, SUPPORT[category]


def _boolean_support_style(
    row: dict[str, str] | None,
    field: str,
    label: str,
    body_fill: str = "#33363d",
) -> tuple[str, str, str]:
    category = "-" if row is None else row.get(field, "-")
    styles = {
        "yes": ("✓", "#4ade80"),
        "no": ("✗", "#f87171"),
        "-": ("-", "#a5abb5"),
    }
    if category not in styles:
        raise MetricsError(
            f"malformed {label} support category {category!r}"
        )
    text, foreground = styles[category]
    return text, body_fill, foreground


def _signed_zero_style(
    row: dict[str, str] | None,
    body_fill: str = "#33363d",
) -> tuple[str, str, str]:
    return _boolean_support_style(
        row, "signed_zero_support", "signed-zero", body_fill,
    )


def _subnormal_style(
    row: dict[str, str] | None,
    body_fill: str = "#33363d",
) -> tuple[str, str, str]:
    return _boolean_support_style(
        row, "subnormal_support", "subnormal", body_fill,
    )


def _column_specs(consumer_mode: str) -> tuple[tuple[str, tuple[str, ...], int], ...]:
    return (
        COLUMN_SPECS + (SUBNORMAL_COLUMN_SPEC,)
        if consumer_mode == "fastmath"
        else COLUMN_SPECS
    )


def _display_row(
    dataset: Dataset,
    target: Target,
    precision: str,
    group: str,
    operation: str,
    implementation: str,
    row: dict[str, str] | None,
    *,
    normal_only: bool,
) -> dict[str, str] | None:
    """Return report values while retaining subnormal evidence separately."""

    if row is None:
        return None
    display = dict(row)
    details = [
        detail
        for (
            detail_target, detail_precision, detail_group,
            detail_operation, detail_implementation, _,
        ), detail in dataset.accuracy.items()
        if (
            detail_target, detail_precision, detail_group,
            detail_operation, detail_implementation,
        ) == (target, precision, group, operation, implementation)
    ]
    subnormal = [
        detail for detail in details
        if detail["domain"] == SUBNORMAL_DOMAIN
    ]
    if implementation == "fltx" and subnormal:
        display["subnormal_support"] = (
            "yes" if all(detail["pass"] == "yes" for detail in subnormal)
            else "no"
        )
    else:
        display["subnormal_support"] = "-"

    primary = [
        detail for detail in details
        if detail["domain"] != SUBNORMAL_DOMAIN
    ]
    if not normal_only or not subnormal or not primary:
        return display

    counts = [int(detail["samples"]) for detail in primary]
    exact = [math.isinf(float(detail["mean_bits"])) for detail in primary]
    means = [float(detail["mean_bits"]) for detail in primary]
    total = sum(counts)
    nominal_bits = next(
        item.nominal_bits
        for item in IMPLEMENTATIONS[precision]
        if item.id == implementation
    )
    pooled = [
        nominal_bits + 32.0 if is_exact else mean
        for mean, is_exact in zip(means, exact)
    ]
    display.update({
        "samples": str(total),
        "mean_bits": (
            "inf" if all(exact)
            else str(sum(value * count for value, count in zip(pooled, counts)) / total)
        ),
        "p01_bits": str(min(float(detail["p01_bits"]) for detail in primary)),
        "worst_bits": str(min(float(detail["worst_bits"]) for detail in primary)),
        "domains_passed": str(sum(detail["pass"] == "yes" for detail in primary)),
        "domains_total": str(len(primary)),
        "min_margin_bits": str(min(float(detail["margin_bits"]) for detail in primary)),
        "accuracy_pass": (
            "yes" if all(detail["pass"] == "yes" for detail in primary)
            else "no"
        ),
    })
    return display


def _accuracy_style(
    value: str | None,
    body_fill: str,
    layout: str = "full",
) -> tuple[str, str, str]:
    exact = bool(value) and math.isinf(float(value))
    text = (
        "-"
        if value is None
        else (
            _fmt_bits_compact(value)
            if layout == "compact"
            else _fmt_bits(value)
        )
    )
    exact_text = (
        COMPACT_EXACT_TEXT
        if layout == "compact"
        else EXACT_TEXT
    )
    return text, body_fill, exact_text if exact else "#e7e9ee"


def _benchmark_text(
    row: dict[str, str] | None,
    implementation: str,
    layout: str = "full",
) -> str:
    if row is None or not row["ns_iter"]:
        return "-"
    if layout == "compact":
        return f"{_fmt_ns_compact(row['ns_iter'])} ns"
    timing = f"{_fmt_ns(row['ns_iter'])} ns"
    speed_ratio = _speed_vs_fltx(row, implementation)
    if speed_ratio is None:
        return timing
    return f"{timing} · {_fmt_ratio(speed_ratio)}"


def _benchmark_style(
    row: dict[str, str] | None,
    implementation: str,
    body_fill: str,
    layout: str = "full",
) -> tuple[str, str, str]:
    text = _benchmark_text(row, implementation, layout)
    if text == "-":
        return text, body_fill, "#a5abb5"
    return text, body_fill, "#e7e9ee"


def _fastest_competitor(
    dataset: Dataset,
    target: Target,
    precision: str,
    group: str,
    operation: str,
) -> tuple[dict[str, str], float] | None:
    candidates = []
    for implementation in dataset.implementations[(target, precision)]:
        if implementation == "fltx":
            continue
        row = dataset.canonical.get(
            (target, precision, group, operation, implementation)
        )
        if row is None or not row["ns_iter"] or not row["speed_ratio"]:
            continue
        ratio = float(row["speed_ratio"])
        if ratio <= 0:
            raise MetricsError("benchmark time ratio must be positive")
        candidates.append((row, ratio))
    return min(candidates, key=lambda candidate: candidate[1]) if candidates else None


def _benchmark_spans(
    row: dict[str, str] | None,
    implementation: str,
    *,
    fltx_speed_ratio: float | None = None,
    layout: str = "full",
) -> tuple[tuple[str, str], ...]:
    """Keep the timing neutral while colouring only the relative speed."""

    text = _benchmark_text(row, implementation, layout)
    if text == "-":
        return ((text, "#a5abb5"),)
    speed_ratio = (
        fltx_speed_ratio
        if implementation == "fltx"
        else _speed_vs_fltx(row, implementation)
    )
    if layout == "compact":
        text_color = (
            _ratio_color(speed_ratio)
            if speed_ratio is not None
            else "#e7e9ee"
        )
        if text.endswith(" ns"):
            return (
                (text[:-3], text_color),
                ("ns", COMPACT_BENCHMARK_UNIT_TEXT),
            )
        return ((
            text,
            text_color,
        ),)
    if implementation == "fltx" and fltx_speed_ratio is not None:
        return ((text, _ratio_color(fltx_speed_ratio)),)
    if speed_ratio is None:
        return ((text, "#e7e9ee"),)
    timing, ratio = text.split(" · ", 1)
    return (
        (timing, "#e7e9ee"),
        (" · ", BENCHMARK_SEPARATOR_TEXT),
        (ratio, _ratio_color(speed_ratio)),
    )


def _tooltip(
    dataset: Dataset,
    target: Target,
    precision: str,
    group: str,
    operation: str,
    implementation: str,
    row: dict[str, str] | None,
) -> str:
    if row is None:
        return f"{implementation}: {operation} is unavailable"
    lines = [
        row["implementation_label"],
        f"API: {row['api']}",
    ]
    if row["mean_bits"]:
        lines.extend((
            f"mean: {_fmt_bits(row['mean_bits'])} bits",
            f"p01: {_fmt_bits(row['p01_bits'])} bits",
            f"worst: {_fmt_bits(row['worst_bits'])} bits",
            f"domains: {row['domains_passed']}/{row['domains_total']}",
            f"minimum threshold margin: {_fmt_bits(row['min_margin_bits'])} bits",
            f"special values: {row['special_support']}",
            f"signed zero: {row.get('signed_zero_support', '-')}",
            f"subnormal support: {row.get('subnormal_support', '-')}",
        ))
        details = sorted(
            (
                domain,
                detail,
            )
            for (
                detail_target, detail_precision, detail_group,
                detail_operation, detail_implementation, domain,
            ), detail in dataset.accuracy.items()
            if (
                detail_target, detail_precision, detail_group,
                detail_operation, detail_implementation,
            ) == (target, precision, group, operation, implementation)
        )
        for domain, detail in details:
            fltx = dataset.accuracy.get(
                (target, precision, group, operation, "fltx", domain)
            )
            required = "-" if fltx is None else fltx["required_worst_bits"]
            margin = "-"
            if required and detail["worst_bits"]:
                margin = _fmt_bits(
                    str(float(detail["worst_bits"]) - float(required))
                )
            lines.append(
                f"{domain}: mean {_fmt_bits(detail['mean_bits'])}, "
                f"p01 {_fmt_bits(detail['p01_bits'])}, "
                f"worst {_fmt_bits(detail['worst_bits'])}, "
                f"required {_fmt_bits(required)}, margin {margin}, "
                f"n={detail['samples']}, seed={detail['seed']}"
            )
    if row["ns_iter"]:
        benchmark = f"benchmark: {_fmt_ns(row['ns_iter'])} ns/iteration"
        speed_ratio = _speed_vs_fltx(row, implementation)
        if speed_ratio is not None:
            benchmark += f", speed vs FLTX {_fmt_ratio(speed_ratio)}"
        elif implementation == "fltx":
            fastest = _fastest_competitor(
                dataset, target, precision, group, operation,
            )
            if fastest is None:
                benchmark += " (no available competitor)"
            else:
                competitor, _ = fastest
                benchmark += (
                    f", speed vs fastest competitor "
                    f"{competitor['implementation_short']} "
                    f"({competitor['implementation_label']}): "
                    f"{competitor['speed_ratio']}×"
                )
        else:
            benchmark += " (FLTX baseline)"
        lines.append(benchmark)
    return "\n".join(lines)


def _rect(
    parts: list[str],
    x: int,
    y: int,
    width: int,
    height: int,
    fill: str,
) -> None:
    parts.append(
        f'<rect x="{x}" y="{y}" width="{width}" height="{height}" '
        f'fill="{fill}"/>'
    )


def _line(
    parts: list[str],
    x1: int,
    y1: int,
    x2: int,
    y2: int,
    *,
    stroke: str = GRID_REGULAR,
    stroke_width: int = 1,
) -> None:
    parts.append(
        f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" '
        f'stroke="{stroke}" stroke-width="{stroke_width}" '
        f'shape-rendering="crispEdges"/>'
    )


def _text(
    parts: list[str],
    x: float,
    y: float,
    value: str,
    *,
    fill: str = "#e7e9ee",
    anchor: str = "middle",
    weight: str = "normal",
    size: int = 11,
    font_size_increment: int = 0,
    snap_font_size: bool = False,
) -> None:
    rendered_size = _rendered_font_size(
        size, font_size_increment, snap_font_size,
    )
    parts.append(
        f'<text x="{x}" y="{y}" fill="{fill}" text-anchor="{anchor}" '
        f'font-size="{rendered_size:g}" font-weight="{weight}">'
        f'{html.escape(value)}</text>'
    )


def _infinity_mark(
    parts: list[str],
    x: float,
    y: float,
    *,
    stroke: str = COMPACT_EXACT_TEXT,
) -> None:
    """Draw a centered, subtly variable-width compact exact marker."""

    base = (
        f'M {x:g} {y:g} '
        f'C {x - 2.2:g} {y - 2.3:g} {x - 3.7:g} {y - 4.5:g} '
        f'{x - 6.4:g} {y - 4.5:g} '
        f'C {x - 11:g} {y - 4.5:g} {x - 11:g} {y + 4.5:g} '
        f'{x - 6.4:g} {y + 4.5:g} '
        f'C {x - 3.7:g} {y + 4.5:g} {x - 2.2:g} {y + 2.3:g} '
        f'{x:g} {y:g} '
        f'C {x + 2.2:g} {y - 2.3:g} {x + 3.7:g} {y - 4.5:g} '
        f'{x + 6.4:g} {y - 4.5:g} '
        f'C {x + 11:g} {y - 4.5:g} {x + 11:g} {y + 4.5:g} '
        f'{x + 6.4:g} {y + 4.5:g} '
        f'C {x + 3.7:g} {y + 4.5:g} {x + 2.2:g} {y + 2.3:g} '
        f'{x:g} {y:g} Z'
    )
    left_lobe = (
        f'M {x - 4:g} {y - 4.3:g} '
        f'C {x - 8:g} {y - 5.6:g} {x - 10.5:g} {y - 3.6:g} '
        f'{x - 10.5:g} {y:g} '
        f'C {x - 10.5:g} {y + 3.6:g} {x - 8:g} {y + 5.6:g} '
        f'{x - 4:g} {y + 4.3:g}'
    )
    right_lobe = (
        f'M {x + 4:g} {y - 4.3:g} '
        f'C {x + 8:g} {y - 5.6:g} {x + 10.5:g} {y - 3.6:g} '
        f'{x + 10.5:g} {y:g} '
        f'C {x + 10.5:g} {y + 3.6:g} {x + 8:g} {y + 5.6:g} '
        f'{x + 4:g} {y + 4.3:g}'
    )
    descending_crossing = (
        f'M {x - 3.2:g} {y - 3.7:g} '
        f'C {x - 1.8:g} {y - 2.3:g} {x - 0.7:g} {y - 0.8:g} '
        f'{x:g} {y:g} '
        f'C {x + 0.7:g} {y + 0.8:g} {x + 1.8:g} {y + 2.3:g} '
        f'{x + 3.2:g} {y + 3.7:g}'
    )
    ascending_crossing = (
        f'M {x - 3.2:g} {y + 3.7:g} '
        f'C {x - 1.8:g} {y + 2.3:g} {x - 0.7:g} {y + 0.8:g} '
        f'{x:g} {y:g} '
        f'C {x + 0.7:g} {y - 0.8:g} {x + 1.8:g} {y - 2.3:g} '
        f'{x + 3.2:g} {y - 3.7:g}'
    )
    parts.append(
        f'<g class="exact-mark" fill="none" stroke="{stroke}" '
        f'transform="translate({x:g} {y:g}) '
        f'scale({COMPACT_EXACT_SCALE:g}) translate({-x:g} {-y:g})" '
        f'stroke-linecap="round" stroke-linejoin="round">'
        f'<path d="{base}" '
        f'stroke-width="{COMPACT_EXACT_STROKE_WIDTH:g}"/>'
        f'<path d="{left_lobe}" stroke-width="1.65"/>'
        f'<path d="{right_lobe}" stroke-width="1.65"/>'
        f'<path d="{descending_crossing}" stroke-width="1.45"/>'
        f'<path d="{ascending_crossing}" stroke-width="1.45"/>'
        f'</g>'
    )


def _text_spans(
    parts: list[str],
    x: float,
    y: float,
    spans: tuple[tuple[str, str], ...],
    *,
    anchor: str = "middle",
    weight: str = "normal",
    size: int = 11,
    font_size_increment: int = 0,
    span_dx: tuple[float, ...] = (),
    snap_font_size: bool = False,
) -> None:
    rendered_size = _rendered_font_size(
        size, font_size_increment, snap_font_size,
    )
    rendered_spans = []
    for index, (value, fill) in enumerate(spans):
        dx = span_dx[index] if index < len(span_dx) else 0
        dx_attribute = f' dx="{dx:g}"' if dx else ""
        rendered_spans.append(
            f'<tspan fill="{fill}"{dx_attribute}>'
            f'{html.escape(value)}</tspan>'
        )
    parts.append(
        f'<text x="{x}" y="{y}" text-anchor="{anchor}" '
        f'font-size="{rendered_size:g}" font-weight="{weight}">'
        + "".join(rendered_spans)
        + "</text>"
    )


def _text_lines(
    parts: list[str],
    x: float,
    y: int,
    height: int,
    lines: tuple[str, ...],
    *,
    fill: str = "#e7e9ee",
    weight: str = "normal",
    size: int = 9,
    line_height: int = 14,
    font_size_increment: int = 0,
    snap_font_size: bool = False,
) -> None:
    if not lines:
        return
    rendered_size = _rendered_font_size(
        size, font_size_increment, snap_font_size,
    )
    text_block_height = rendered_size + (len(lines) - 1) * line_height
    first_baseline = (
        y + (height - text_block_height) / 2 + rendered_size * 0.8
    )
    for index, line in enumerate(lines):
        _text(
            parts,
            x,
            first_baseline + index * line_height,
            line,
            fill=fill,
            weight=weight,
            size=size,
            font_size_increment=font_size_increment,
            snap_font_size=snap_font_size,
        )


def _implementation_order(dataset: Dataset, target: Target, precision: str) -> tuple[str, ...]:
    declared = dataset.implementations[(target, precision)]
    return ("fltx",) + tuple(item for item in declared if item != "fltx")


def _implementation_tint(
    implementation: str,
    implementations: tuple[str, ...],
) -> tuple[str, str]:
    if implementation == "fltx":
        return FLTX_TINT
    competitors = tuple(item for item in implementations if item != "fltx")
    return IMPLEMENTATION_TINTS[
        competitors.index(implementation) % len(IMPLEMENTATION_TINTS)
    ]


def _column_header_lines(
    key: str,
    lines: tuple[str, ...],
    layout: str,
    implementation: str,
) -> tuple[str, ...]:
    if layout == "compact":
        return {
            "mean": ("avg",),
            "worst": ("min",),
            "benchmark": ("(ns)",),
        }.get(key, lines)
    if key == "benchmark" and implementation == "fltx":
        return ("time",)
    return lines


def _column_widths(
    dataset: Dataset,
    target: Target,
    precision: str,
    operations: list[tuple[str, str]],
    implementation: str,
    layout: str = "full",
) -> tuple[int, ...]:
    column_specs = _column_specs(dataset.consumer_mode)
    cell_padding = (
        COMPACT_CELL_PADDING if layout == "compact" else CELL_PADDING
    )
    visible = {}
    for key, lines, _ in column_specs:
        visible[key] = list(
            _column_header_lines(key, lines, layout, implementation)
        )
    bits_formatter = (
        _fmt_bits_compact if layout == "compact" else _fmt_bits
    )
    for group, operation in operations:
        row = dataset.canonical.get(
            (target, precision, group, operation, implementation)
        )
        row = _display_row(
            dataset, target, precision, group, operation, implementation, row,
            normal_only=dataset.consumer_mode == "fastmath",
        )
        visible["mean"].append(
            "-" if row is None else bits_formatter(row["mean_bits"])
        )
        visible["worst"].append(
            "-" if row is None else bits_formatter(row["worst_bits"])
        )
        visible["pass"].append(_domain_style(row)[0])
        visible["benchmark"].append(
            _benchmark_text(row, implementation, layout)
        )
        visible["special"].append(_special_style(row)[0])
        visible["signed_zero"].append(_signed_zero_style(row)[0])
        if dataset.consumer_mode == "fastmath":
            visible["subnormal"].append(_subnormal_style(row)[0])

    def required_width(lines: tuple[str, ...] | list[str]) -> int:
        return (
            math.ceil(
                max(
                    (_estimated_text_width(value, layout, 9) for value in lines),
                    default=0,
                )
            )
            + cell_padding
        )

    widths = [
        max(
            (
                (
                    28 if key in {"mean", "worst"} else minimum
                )
                - (CELL_PADDING - COMPACT_CELL_PADDING)
                - (
                    COMPACT_SPECIAL_WIDTH_REDUCTION
                    if key == "special"
                    else 0
                )
                if layout == "compact"
                else minimum
            ),
            required_width(visible[key]),
        )
        for key, _, minimum in column_specs
    ]

    # Second-level headers span one or more leaf columns. Apply their width
    # constraints after sizing the leaf content so each retains real padding.
    accuracy_required = required_width(
        ("bits", "accurate") if layout == "compact" else ("bits accurate",)
    )
    accuracy_deficit = max(0, accuracy_required - widths[0] - widths[1])
    widths[0] += (accuracy_deficit + 1) // 2
    widths[1] += accuracy_deficit // 2
    if layout == "compact":
        widths[0] += (COMPACT_ACCURACY_EXTRA_WIDTH + 1) // 2
        widths[1] += COMPACT_ACCURACY_EXTRA_WIDTH // 2
    widths[2] = max(widths[2], required_width(("domain",)))
    benchmark_group_header = (
        ("bench",) if layout == "compact" else ("performance",)
    )
    widths[3] = max(widths[3], required_width(benchmark_group_header))
    widths[4] = max(widths[4], required_width(("Inf/", "NaN")))
    widths[5] = max(widths[5], required_width(("±0",)))
    if dataset.consumer_mode == "fastmath":
        widths[6] = max(widths[6], required_width(("Subnorm",)))
    return tuple(widths)


def _operation_width(
    operations: list[tuple[str, str]],
    layout: str = "full",
) -> int:
    font_size = 9 if layout == "compact" else 10
    cell_padding = (
        COMPACT_CELL_PADDING if layout == "compact" else CELL_PADDING
    )
    longest = max(
        len("operation"),
        *(len(operation) for _, operation in operations),
    )
    return (
        math.ceil(
            longest * _estimated_character_width(layout, font_size)
        )
        + cell_padding
        + 4
    )


def _block_x(
    index: int,
    operation_width: int,
    block_widths: tuple[int, ...],
    *,
    margin: int = MARGIN,
    table_gap: int = TABLE_GAP,
) -> int:
    return (
        margin + operation_width + table_gap +
        sum(width + table_gap for width in block_widths[:index])
    )


def _grouped_operations(
    operations: list[tuple[str, str]],
) -> list[tuple[str, list[tuple[str, str]]]]:
    grouped: list[tuple[str, list[tuple[str, str]]]] = []
    for group, operation in operations:
        report_group = _report_group(group)
        if not grouped or grouped[-1][0] != report_group:
            grouped.append((report_group, []))
        grouped[-1][1].append((group, operation))
    return grouped


def render_overview(
    dataset: Dataset,
    target: Target,
    precision: str,
    layout: str = "full",
) -> str:
    if layout not in {"full", "compact"}:
        raise ValueError(f"unsupported overview layout {layout!r}")
    margin = COMPACT_MARGIN if layout == "compact" else MARGIN
    table_gap = COMPACT_TABLE_GAP if layout == "compact" else TABLE_GAP
    font_size_increment = (
        COMPACT_FONT_SIZE_INCREMENT if layout == "compact" else 0
    )
    snap_font_size = layout == "compact"
    header_heights = (
        COMPACT_HEADER_HEIGHTS
        if layout == "compact"
        else HEADER_HEIGHTS
    )
    title_height = 0 if layout == "compact" else TITLE_HEIGHT
    footer_height = 0 if layout == "compact" else FOOTER_HEIGHT
    add_text = partial(
        _text,
        font_size_increment=font_size_increment,
        snap_font_size=snap_font_size,
    )
    add_text_spans = partial(
        _text_spans,
        font_size_increment=font_size_increment,
        snap_font_size=snap_font_size,
    )
    add_text_lines = partial(
        _text_lines,
        font_size_increment=font_size_increment,
        snap_font_size=snap_font_size,
    )
    implementations = _implementation_order(dataset, target, precision)
    keys = {
        (group, operation)
        for (
            row_target, row_precision, group, operation, implementation,
        ) in dataset.canonical
        if row_target == target and row_precision == precision
        and (group, operation) not in REPORT_EXCLUDED_OPERATIONS
    }
    operations = sorted(
        keys,
        key=lambda value: (
            GROUP_ORDER.get(value[0], len(GROUP_ORDER)),
            value[0],
            OPERATION_ORDER.get(value[0], {}).get(
                value[1], len(OPERATION_ORDER.get(value[0], {})),
            ),
            value[1],
        ),
    )
    if not operations:
        raise MetricsError(f"{target.label} {precision}: no canonical operations")

    column_widths_by_implementation = {
        implementation: _column_widths(
            dataset, target, precision, operations, implementation, layout,
        )
        for implementation in implementations
    }
    block_widths = tuple(
        sum(column_widths_by_implementation[implementation])
        for implementation in implementations
    )
    operation_width = _operation_width(operations, layout)
    width = (
        margin * 2 + operation_width +
        len(implementations) * table_gap + sum(block_widths)
    )
    title_mode = (
        " consumer fast-math" if dataset.consumer_mode == "fastmath" else ""
    )
    title = (
        f"{PRECISION_TYPE_NAMES[precision]}{title_mode} metrics overview — "
        f"{target.label}"
    )
    subtitle = (
        "Accuracy excludes the subnormal domain; support is reported separately. "
        "Speed is relative to FLTX."
        if dataset.consumer_mode == "fastmath"
        else
        "Accuracy is MPFR-relative; domain pass counts use FLTX release "
        "thresholds; speed is implementation ÷ FLTX."
    )
    if layout == "full":
        character_width = _estimated_character_width("full", 10)
        width = max(
            width,
            math.ceil(len(title) * character_width * 1.5) + margin * 2,
            math.ceil(len(subtitle) * character_width) + margin * 2,
        )
    header_height = sum(header_heights)
    body_y = title_height + header_height
    operation_groups = _grouped_operations(operations)
    body_height = (
        ROW_HEIGHT * len(operations) +
        GROUP_ROW_HEIGHT * len(operation_groups)
    )
    height = body_y + body_height + footer_height
    parts = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
        f'viewBox="0 0 {width} {height}">',
        _font_css(layout),
        f'<rect width="{width}" height="{height}" fill="#0f1115"/>',
    ]
    if layout == "full":
        add_text(
            parts, margin, 23,
            title,
            anchor="start", weight="bold", size=15,
        )
        add_text(
            parts, margin, 42,
            subtitle,
            anchor="start", fill="#aeb4bf", size=10,
        )

    header_y = title_height
    _rect(parts, margin, header_y, operation_width, header_height, "#2B2B2B")
    add_text(
        parts, margin + operation_width / 2,
        header_y + header_height / 2 + 4,
        "operation", weight="bold", size=10,
    )

    labels: dict[str, tuple[str, str]] = {}
    for implementation in implementations:
        candidates = [
            row for (
                row_target, row_precision, _, _, row_implementation,
            ), row in dataset.canonical.items()
            if (
                row_target == target and row_precision == precision and
                row_implementation == implementation
            )
        ]
        if not candidates:
            raise MetricsError(
                f"{target.label} {precision}: missing implementation {implementation}"
            )
        labels[implementation] = (
            candidates[0]["implementation_short"],
            candidates[0]["implementation_label"],
        )

    for index, implementation in enumerate(implementations):
        column_widths = column_widths_by_implementation[implementation]
        block_width = block_widths[index]
        x = _block_x(
            index,
            operation_width,
            block_widths,
            margin=margin,
            table_gap=table_gap,
        )
        header_fill, _ = _implementation_tint(
            implementation, implementations
        )
        parts.append(
            f'<g class="implementation-table" '
            f'data-implementation="{html.escape(implementation)}" '
            f'data-x="{x}" data-width="{block_width}">'
        )
        _rect(parts, x, header_y, block_width, header_heights[0], header_fill)
        label = (
            f"fltx ({PRECISION_TYPE_NAMES[precision]})"
            if implementation == "fltx"
            else PRIMARY_HEADER_LABELS.get(
                labels[implementation][0],
                labels[implementation][0],
            )
        )
        add_text(
            parts, x + block_width / 2, header_y + 16,
            label,
            weight="normal" if layout == "compact" else "bold",
            fill="#ffffff",
            size=10,
        )

        second_y = header_y + header_heights[0]
        accuracy_width = column_widths[0] + column_widths[1]
        accuracy_header = (
            ("bits", "accurate") if layout == "compact" else ("bits accurate",)
        )
        benchmark_header = (
            ("bench", "(ns)")
            if layout == "compact"
            else ("performance",)
        )
        groups = (
            (accuracy_header, accuracy_width, False),
            (("domain",), column_widths[2], False),
            (
                benchmark_header,
                column_widths[3],
                layout == "compact",
            ),
            (("Inf/", "NaN"), column_widths[4], True),
            (("±0",), column_widths[5], True),
        )
        if dataset.consumer_mode == "fastmath":
            groups += ((("Subnorm",), column_widths[6], True),)
        group_x = x
        for group_lines, group_width, spans_subheader in groups:
            group_height = (
                header_heights[1] + header_heights[2]
                if spans_subheader
                else header_heights[1]
            )
            _rect(parts, group_x, second_y, group_width, group_height, header_fill)
            add_text_lines(
                parts,
                group_x + group_width / 2,
                second_y,
                group_height,
                group_lines,
                size=9,
            )
            group_x += group_width

        third_y = second_y + header_heights[1]
        column_x = x
        for (key, header_lines, _), column_width in zip(
            _column_specs(dataset.consumer_mode), column_widths
        ):
            if (
                key in {"special", "signed_zero", "subnormal"} or
                (
                    layout == "compact" and
                    key == "benchmark"
                )
            ):
                column_x += column_width
                continue
            _rect(parts, column_x, third_y, column_width, header_heights[2], header_fill)
            add_text_lines(
                parts,
                column_x + column_width / 2,
                third_y,
                header_heights[2],
                _column_header_lines(
                    key, header_lines, layout, implementation,
                ),
                size=9,
            )
            column_x += column_width
        parts.append("</g>")

    group_spans: list[tuple[int, int, int]] = []
    y = body_y
    row_index = 0
    for report_group, group_operations in operation_groups:
        group_y = y
        parts.append(
            f'<g class="group-row" data-group="{html.escape(report_group)}">'
        )
        _rect(
            parts, margin, group_y, operation_width,
            GROUP_ROW_HEIGHT, GROUP_ROW_FILL,
        )
        add_text(
            parts,
            margin + 7,
            group_y + 13,
            GROUP_LABELS.get(
                report_group,
                report_group.replace("_", " ").title(),
            ),
            anchor="start",
            fill=GROUP_ROW_TEXT,
            weight="normal",
            size=9 if layout == "compact" else 10,
        )
        for index in range(len(implementations)):
            x = _block_x(
                index,
                operation_width,
                block_widths,
                margin=margin,
                table_gap=table_gap,
            )
            _rect(
                parts, x, group_y, block_widths[index],
                GROUP_ROW_HEIGHT, GROUP_ROW_FILL,
            )
        parts.append("</g>")
        y += GROUP_ROW_HEIGHT
        operations_y = y

        for group, operation in group_operations:
            fastest = _fastest_competitor(
                dataset, target, precision, group, operation,
            )
            fltx_speed_ratio = None if fastest is None else fastest[1]
            parts.append('<g class="body-row">')
            _rect(
                parts, margin, y, operation_width, ROW_HEIGHT,
                OPERATION_ROW_FILLS[row_index % 2],
            )
            add_text(
                parts,
                margin + 7 - (
                    COMPACT_OPERATION_TEXT_SHIFT
                    if layout == "compact"
                    else 0
                ),
                y + 15,
                operation,
                anchor="start", fill="#f1f2f4",
                size=9 if layout == "compact" else 10,
            )

            for index, implementation in enumerate(implementations):
                column_widths = column_widths_by_implementation[implementation]
                x = _block_x(
                    index,
                    operation_width,
                    block_widths,
                    margin=margin,
                    table_gap=table_gap,
                )
                row = dataset.canonical.get(
                    (target, precision, group, operation, implementation)
                )
                row = _display_row(
                    dataset,
                    target,
                    precision,
                    group,
                    operation,
                    implementation,
                    row,
                    normal_only=dataset.consumer_mode == "fastmath",
                )
                body_fill = OPERATION_ROW_FILLS[row_index % 2]
                tooltip = _tooltip(
                    dataset,
                    target,
                    precision,
                    group,
                    operation,
                    implementation,
                    row,
                )
                values: list[tuple[str, str, str]] = [
                    _accuracy_style(
                        None if row is None else row["mean_bits"],
                        body_fill,
                        layout,
                    ),
                    _accuracy_style(
                        None if row is None else row["worst_bits"],
                        body_fill,
                        layout,
                    ),
                    _domain_style(row, body_fill),
                    _benchmark_style(row, implementation, body_fill, layout),
                    _special_style(row, body_fill),
                    _signed_zero_style(row, body_fill),
                ]
                if dataset.consumer_mode == "fastmath":
                    values.append(_subnormal_style(row, body_fill))
                parts.append("<g>")
                parts.append(f"<title>{html.escape(tooltip)}</title>")
                column_x = x
                for column_index, (
                    column_width, (value, fill, text_fill),
                ) in enumerate(zip(column_widths, values)):
                    _rect(
                        parts, column_x, y, column_width, ROW_HEIGHT, fill,
                    )
                    if column_index == 3:
                        add_text_spans(
                            parts,
                            column_x + column_width / 2,
                            y + 15,
                            _benchmark_spans(
                                row,
                                implementation,
                                fltx_speed_ratio=(
                                    fltx_speed_ratio
                                    if implementation == "fltx"
                                    else None
                                ),
                                layout=layout,
                            ),
                            size=9,
                            span_dx=(
                                (0, COMPACT_BENCHMARK_UNIT_GAP)
                                if layout == "compact"
                                else ()
                            ),
                        )
                    else:
                        compact_accuracy_marker = (
                            layout == "compact" and
                            column_index in {0, 1}
                        )
                        drawn_exact = (
                            compact_accuracy_marker and
                            value == "∞"
                        )
                        if drawn_exact:
                            _infinity_mark(
                                parts,
                                column_x + column_width / 2,
                                y + ROW_HEIGHT / 2,
                                stroke=text_fill,
                            )
                        else:
                            add_text(
                                parts,
                                column_x + column_width / 2,
                                y + 15,
                                value,
                                fill=text_fill,
                                size=(
                                    COMPACT_REFERENCE_MATCH_FONT_SIZE
                                    if compact_accuracy_marker and value == "="
                                    else 9
                                ),
                            )
                    column_x += column_width
                parts.append("</g>")
            parts.append("</g>")
            y += ROW_HEIGHT
            row_index += 1

        group_spans.append((group_y, operations_y, y))

    grid_bottom = y

    # The operation index and each implementation are separate tables. Draw
    # every grid segment once so adjacent cells never create doubled borders.
    _line(parts, margin, header_y, margin + operation_width, header_y, stroke=GRID_STRONG)
    _line(parts, margin, header_y, margin, grid_bottom, stroke=GRID_STRONG)
    _line(
        parts,
        margin + operation_width,
        header_y,
        margin + operation_width,
        grid_bottom,
        stroke=GRID_STRONG,
    )

    for index, implementation in enumerate(implementations):
        column_widths = column_widths_by_implementation[implementation]
        block_width = block_widths[index]
        x = _block_x(
            index,
            operation_width,
            block_widths,
            margin=margin,
            table_gap=table_gap,
        )
        right = x + block_width
        second_y = header_y + header_heights[0]
        third_y = second_y + header_heights[1]

        _line(parts, x, header_y, right, header_y, stroke=GRID_STRONG)
        _line(parts, x, header_y, x, grid_bottom, stroke=GRID_STRONG)
        _line(parts, right, header_y, right, grid_bottom, stroke=GRID_STRONG)
        _line(parts, x, second_y, right, second_y, stroke=GRID_REGULAR)
        special_x = x + sum(column_widths[:4])
        third_row_right = (
            x + sum(column_widths[:3])
            if layout == "compact"
            else special_x
        )
        _line(
            parts,
            x,
            third_y,
            third_row_right,
            third_y,
            stroke=GRID_REGULAR,
        )
        boundaries = []
        position = x
        for column_width in column_widths[:-1]:
            position += column_width
            boundaries.append(position)

        # Mean/worst is a third-level subdivision. The remaining boundaries
        # also divide second-level groups. Body dividers stop at group rows.
        _line(parts, boundaries[0], third_y, boundaries[0], body_y)
        for boundary in boundaries[1:]:
            _line(parts, boundary, second_y, boundary, body_y)
        for _, operations_y, operations_end in group_spans:
            _line(
                parts,
                boundaries[0],
                operations_y,
                boundaries[0],
                operations_end,
            )
            for boundary in boundaries[1:]:
                _line(
                    parts,
                    boundary,
                    operations_y,
                    boundary,
                    operations_end,
                )

    for group_y, operations_y, operations_end in group_spans:
        for left, right in (
            (margin, margin + operation_width),
            *(
                (
                    _block_x(
                        index,
                        operation_width,
                        block_widths,
                        margin=margin,
                        table_gap=table_gap,
                    ),
                    _block_x(
                        index,
                        operation_width,
                        block_widths,
                        margin=margin,
                        table_gap=table_gap,
                    )
                    + block_widths[index],
                )
                for index in range(len(implementations))
            ),
        ):
            _line(parts, left, group_y, right, group_y, stroke=GRID_STRONG)
            _line(
                parts,
                left,
                operations_y,
                right,
                operations_y,
                stroke=GRID_STRONG,
            )
            operation_y = operations_y + ROW_HEIGHT
            while operation_y < operations_end:
                _line(
                    parts,
                    left,
                    operation_y,
                    right,
                    operation_y,
                    stroke=GRID_REGULAR,
                )
                operation_y += ROW_HEIGHT

    _line(
        parts,
        margin,
        grid_bottom,
        margin + operation_width,
        grid_bottom,
        stroke=GRID_STRONG,
    )
    for index in range(len(implementations)):
        x = _block_x(
            index,
            operation_width,
            block_widths,
            margin=margin,
            table_gap=table_gap,
        )
        _line(
            parts,
            x,
            grid_bottom,
            x + block_widths[index],
            grid_bottom,
            stroke=GRID_STRONG,
        )

    if layout == "full":
        footer_y = grid_bottom + 20
        add_text(
            parts, margin, footer_y,
            "Text: green pass · amber pass within 4 bits · red fail.  Special: Both / Inf / NaN / No / –.",
            anchor="start", fill="#aeb4bf", size=10,
        )
    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=Path(__file__).parents[1] / "data")
    parser.add_argument("--target", type=_target, required=True)
    parser.add_argument("--output", type=Path, default=Path(__file__).parents[1] / "generated")
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
    try:
        args = parse_args(argv)
        dataset = load(args.input, (args.target,), args.consumer_mode)
        args.output.mkdir(parents=True, exist_ok=True)
        mode_suffix = consumer_mode_suffix(args.consumer_mode)
        layout_suffix = "_compact" if args.layout == "compact" else ""
        for precision in PRECISIONS:
            path = (
                args.output /
                f"{args.target.platform}_{args.target.architecture}_"
                f"{args.target.compiler}_{precision}"
                f"{mode_suffix}_overview{layout_suffix}.svg"
            )
            path.write_text(
                render_overview(dataset, args.target, precision, args.layout),
                encoding="utf-8",
                newline="\n",
            )
            print(path)
        return 0
    except (MetricsError, OSError, ValueError) as error:
        print(f"overview failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
