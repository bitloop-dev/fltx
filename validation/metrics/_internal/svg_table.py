#!/usr/bin/env python3
"""Small, dependency-free SVG table renderer used by metrics."""

from __future__ import annotations

from dataclasses import dataclass, field
from html import escape
from pathlib import Path


PAGE = "#f5f7fb"
INK = "#111827"
MUTED = "#4b5563"
GRID = "#000000"
HEADER = "#3b4b63"
GROUP = "#dbe2ea"
WHITE = "#ffffff"
FONT_SCALE = 1.2


@dataclass(frozen=True)
class Column:
    heading: str
    subheading: str = ""


@dataclass(frozen=True)
class Cell:
    lines: tuple[str, ...] = ()
    fill: str = WHITE
    foreground: str = INK
    tooltip: str = ""


@dataclass(frozen=True)
class Row:
    group: str
    label: str
    cells: tuple[Cell, ...]


@dataclass(frozen=True)
class Table:
    title: str
    description: str
    columns: tuple[Column, ...]
    rows: tuple[Row, ...]
    legend: tuple[str, ...] = field(default_factory=tuple)


def _text(
    x: float,
    y: float,
    value: str,
    *,
    size: int = 12,
    color: str = INK,
    weight: int = 400,
    anchor: str = "start",
) -> str:
    rendered_size = size * FONT_SCALE
    return (
        f'<text x="{x:g}" y="{y:g}" fill="{color}" font-size="{rendered_size:g}" '
        f'font-weight="{weight}" text-anchor="{anchor}" '
        'font-family="Segoe UI, Arial, sans-serif">'
        f"{escape(value)}</text>"
    )


def _rect(x: float, y: float, width: float, height: float, fill: str) -> str:
    return (
        f'<rect x="{x:g}" y="{y:g}" width="{width:g}" height="{height:g}" '
        f'fill="{fill}" stroke="{GRID}" stroke-width="1"/>'
    )


def render(table: Table) -> str:
    """Render a compact table. Text and symbols always carry the color meaning."""

    label_width = max(220, min(430, 18 + max([12] + [len(r.label) for r in table.rows]) * 7))
    column_width = 142
    margin = 18
    title_height = 78 + len(table.legend) * 16
    header_height = 52
    group_height = 28
    def row_height(row: Row) -> int:
        line_count = max((len(cell.lines) for cell in row.cells), default=1)
        return max(48, 16 + line_count * 17)
    groups = []
    for row in table.rows:
        if row.group not in groups:
            groups.append(row.group)
    width = margin * 2 + label_width + max(1, len(table.columns)) * column_width
    height = (
        margin * 2
        + title_height
        + header_height
        + len(groups) * group_height
        + sum(row_height(row) for row in table.rows)
    )
    parts = [
        (
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
            f'viewBox="0 0 {width} {height}" role="img" aria-labelledby="title desc">'
        ),
        f'<title id="title">{escape(table.title)}</title>',
        f'<desc id="desc">{escape(table.description)}</desc>',
        f'<rect width="{width}" height="{height}" fill="{PAGE}"/>',
        _text(margin, margin + 26, table.title, size=24, weight=700),
        _text(margin, margin + 50, table.description, size=12, color=MUTED),
    ]
    for index, line in enumerate(table.legend):
        parts.append(_text(margin, margin + 70 + index * 16, line, size=11, color=MUTED))

    y = margin + title_height
    parts.append(_rect(margin, y, label_width, header_height, HEADER))
    parts.append(_text(margin + 12, y + 31, "Operation", size=13, color=WHITE, weight=700))
    x = margin + label_width
    for column in table.columns:
        parts.append(_rect(x, y, column_width, header_height, HEADER))
        parts.append(
            _text(
                x + column_width / 2,
                y + 22,
                column.heading,
                size=12,
                color=WHITE,
                weight=700,
                anchor="middle",
            )
        )
        if column.subheading:
            parts.append(
                _text(
                    x + column_width / 2,
                    y + 40,
                    column.subheading,
                    size=10,
                    color="#e2e8f0",
                    weight=600,
                    anchor="middle",
                )
            )
        x += column_width
    y += header_height

    for group in groups:
        parts.append(_rect(margin, y, label_width + len(table.columns) * column_width, group_height, GROUP))
        parts.append(_text(margin + 12, y + 19, group, size=12, weight=700))
        y += group_height
        for row in (candidate for candidate in table.rows if candidate.group == group):
            if len(row.cells) != len(table.columns):
                raise ValueError(f"row {row.label!r} has the wrong cell count")
            height = row_height(row)
            parts.append(_rect(margin, y, label_width, height, WHITE))
            parts.append(_text(margin + 12, y + 29, row.label, size=12, weight=600))
            x = margin + label_width
            for cell in row.cells:
                parts.append("<g>")
                if cell.tooltip:
                    parts.append(f"<title>{escape(cell.tooltip)}</title>")
                parts.append(_rect(x, y, column_width, height, cell.fill))
                line_count = len(cell.lines)
                baseline = y + 28 if line_count <= 1 else y + 19
                for line_index, line in enumerate(cell.lines):
                    parts.append(
                        _text(
                            x + column_width / 2,
                            baseline + line_index * 17,
                            line,
                            size=11,
                            color=cell.foreground,
                            weight=650,
                            anchor="middle",
                        )
                    )
                parts.append("</g>")
                x += column_width
            y += height
    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def write(table: Table, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    partial = path.with_suffix(".partial" + path.suffix)
    partial.write_text(render(table), encoding="utf-8", newline="\n")
    partial.replace(path)
