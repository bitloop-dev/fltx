"""Discover existing overview SVGs and wrap them in an offline HTML navigator."""

from __future__ import annotations

import argparse
import json
import math
import os
import re
import sys
import webbrowser
import xml.etree.ElementTree as ET
from pathlib import Path
from urllib.parse import quote

from build_tables import architecture_label, compiler_label, platform_label


DIMENSIONS = (
    ("precision", "Type", ("dd", "qd", "f32", "f64")),
    ("platform", "Platform", ("windows", "linux", "macos", "webassembly")),
    ("architecture", "Architecture", ("x86_64", "arm64", "wasm32")),
    ("compiler", "Compiler", ("MSVC", "ClangCL", "MinGW", "GCC", "Clang", "AppleClang", "Emscripten")),
    ("kind", "Report", ("runtime", "fixed_constexpr")),
    ("mode", "Math mode", ("strict", "fastmath")),
)


def report_identity(path: Path) -> dict[str, str]:
    """Parse the filename contract used by both overview renderers."""
    stem = path.name.removesuffix("_overview.svg")
    mode = "fastmath" if stem.endswith("_fastmath") else "strict"
    stem = stem.removesuffix("_fastmath")
    kind = "fixed_constexpr" if stem.endswith("_fixed_constexpr") else "runtime"
    stem = stem.removesuffix("_fixed_constexpr")
    try:
        target, compiler, precision = stem.rsplit("_", 2)
        platform, architecture = target.split("_", 1)
    except ValueError as error:
        raise ValueError(f"unrecognized overview filename: {path.name}") from error
    if not all((platform, architecture, compiler, precision)):
        raise ValueError(f"unrecognized overview filename: {path.name}")
    return dict(precision=precision, platform=platform, architecture=architecture,
                compiler=compiler, kind=kind, mode=mode)


def _sort_key(report: dict) -> tuple:
    return tuple(
        (order.index(report[key]) if report[key] in order else len(order), report[key])
        for key, _, order in DIMENSIONS
    )


def discover(input_dir: Path, output: Path) -> list[dict]:
    reports = []
    for path in sorted(input_dir.glob("*_overview.svg")):
        if not path.is_file():
            continue
        identity = report_identity(path)
        root = ET.parse(path).getroot()
        if root.tag != "{http://www.w3.org/2000/svg}svg":
            raise ValueError(f"not an SVG report: {path.name}")
        viewbox = root.get("viewBox", "").split()
        if len(viewbox) != 4:
            raise ValueError(f"missing SVG viewBox: {path.name}")
        width, height = map(float, viewbox[2:])
        if not all(math.isfinite(value) and value > 0 for value in (width, height)):
            raise ValueError(f"invalid SVG dimensions: {path.name}")
        # Both overview renderers paint the canvas with their first rectangle.
        canvas = root.find("{http://www.w3.org/2000/svg}rect")
        background = canvas.get("fill", "") if canvas is not None else ""
        if not re.fullmatch(r"#[0-9a-fA-F]{6}", background):
            raise ValueError(f"missing SVG canvas colour: {path.name}")
        try:
            url = quote(Path(os.path.relpath(path, output.parent)).as_posix(), safe="/")
        except ValueError:
            url = path.resolve().as_uri()  # Output on another Windows drive.
        reports.append(dict(identity, file=path.name, url=url, width=width,
                            height=height, background=background))
    if not reports:
        raise ValueError(f"no non-compact overview SVGs found in {input_dir}")
    return sorted(reports, key=_sort_key)


def _label(key: str, value: str) -> str:
    if key == "platform":
        return platform_label(value)
    if key == "architecture":
        return architecture_label(value)
    if key == "compiler":
        return {"ClangCL": "clang-cl", "AppleClang": "AppleClang"}.get(value, compiler_label(value))
    return {"dd": "fdd", "qd": "fqd", "runtime": "Runtime",
            "fixed_constexpr": "Fixed constexpr", "strict": "Strict",
            "fastmath": "Fast math"}.get(value, value)


def build(input_dir: Path, output: Path) -> int:
    reports = discover(input_dir.resolve(), output.resolve())
    dimensions = [dict(key=key, label=label, values=[
        dict(value=value, label=_label(key, value))
        for value in dict.fromkeys(report[key] for report in reports)
    ]) for key, label, _ in DIMENSIONS]
    data = json.dumps(dict(reports=reports, dimensions=dimensions), ensure_ascii=True)
    # Keep filenames inside the JSON script element even if they contain HTML.
    data = data.replace("<", "\\u003c").replace(">", "\\u003e").replace("&", "\\u0026")
    template = Path(__file__).with_name("overview_viewer.html").read_text(encoding="utf-8")
    page = template.replace("__REPORT_DATA__", data).replace("__BACKGROUND__", reports[0]["background"])
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(page, encoding="utf-8", newline="\n")
    return len(reports)


def main(metrics_dir: Path, argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=metrics_dir / "generated" / "overview",
                        help="directory containing overview SVGs (default: generated/overview)")
    parser.add_argument("--output", type=Path,
                        help="HTML destination (default: INPUT/index.html)")
    parser.add_argument("--open", action="store_true", help="open the viewer in your browser")
    args = parser.parse_args(argv)
    output = args.output or args.input / "index.html"
    if output.suffix.lower() != ".html":
        parser.error("--output must be an .html file")
    try:
        count = build(args.input, output)
    except (OSError, ValueError, ET.ParseError) as error:
        print(f"overview viewer failed: {error}", file=sys.stderr)
        return 1
    print(f"Built {output.resolve()} ({count} reports)")
    if args.open:
        webbrowser.open(output.resolve().as_uri())
    return 0
