#!/usr/bin/env python3
"""Repeatable public-header and f256 expression compile telemetry."""

from __future__ import annotations

import argparse
import csv
import shutil
import statistics
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path

from expression_cases import EXPRESSION_STRESS_CASES, expression_for


INCLUDE_HEADERS = (
    "fltx/f128.h",
    "fltx/f256.h",
    "fltx/f128_math.h",
    "fltx/f256_math.h",
    "fltx/math.h",
    "fltx.h",
)


@dataclass(frozen=True)
class Probe:
    category: str
    case: str
    style: str = "none"
    count: int = 0


def probes_for(scope: str, growth_counts: list[int], shape_count: int) -> list[Probe]:
    probes: list[Probe] = []
    if scope in {"summary", "all"}:
        probes.append(Probe("include", "empty"))
        probes.extend(Probe("include", header) for header in INCLUDE_HEADERS)
        for count in growth_counts:
            probes.append(
                Probe("expression-growth", "all_supported_fused_kernels", "eager", count)
            )
            probes.append(
                Probe(
                    "expression-growth",
                    "all_supported_fused_kernels",
                    "expression",
                    count,
                )
            )
    if scope in {"expressions", "all"}:
        for case_id in EXPRESSION_STRESS_CASES:
            probes.append(Probe("expression-shape", case_id, "eager", shape_count))
            probes.append(
                Probe("expression-shape", case_id, "expression", shape_count)
            )
    return probes


def source_for(probe: Probe) -> str:
    if probe.category == "include":
        if probe.case == "empty":
            return "int main() { return 0; }\n"
        return f"#include <{probe.case}>\nint main() {{ return 0; }}\n"

    value_type = "bl::f256" if probe.style == "expression" else "bl::f256_s"
    safe_case = probe.case.replace("-", "_")
    lines = [
        "#include <fltx/f256.h>",
        "#if defined(_MSC_VER)",
        "#define FLTX_COMPILE_NOINLINE __declspec(noinline)",
        "#elif defined(__GNUC__) || defined(__clang__)",
        "#define FLTX_COMPILE_NOINLINE __attribute__((noinline))",
        "#else",
        "#define FLTX_COMPILE_NOINLINE",
        "#endif",
        "namespace fltx_compile_probe {",
    ]
    for index in range(probe.count):
        expression = expression_for(probe.case, index)
        lines.append(
            f"FLTX_COMPILE_NOINLINE {value_type} {probe.style}_{safe_case}_{index}("
            f"{value_type} a, {value_type} b, {value_type} c, {value_type} d, "
            f"{value_type} e, {value_type} f, {value_type} g, {value_type} h, "
            f"double s, double t) {{ return {expression}; }}"
        )
    lines.append("}")
    return "\n".join(lines) + "\n"


def compiler_command(
    compiler: Path,
    compiler_id: str,
    standard: int,
    include: Path,
    source: Path,
    output: Path,
    optimized: bool,
) -> list[str]:
    if compiler_id == "MSVC":
        command = [
            str(compiler),
            "/nologo",
            "/std:c++20" if standard == 20 else "/std:c++latest",
            "/permissive-",
            "/Zc:__cplusplus",
            "/EHsc",
            "/bigobj",
            f"/I{include}",
            "/c",
            str(source),
            f"/Fo{output}",
        ]
        command += ["/O2", "/Ob2", "/DNDEBUG"] if optimized else ["/Od", "/Ob0"]
        return command

    command = [
        str(compiler),
        f"-std=c++{standard}",
        "-pedantic-errors",
        "-I",
        str(include),
        "-c",
        str(source),
        "-o",
        str(output),
    ]
    command += ["-O2", "-DNDEBUG"] if optimized else ["-O0"]
    return command


def measure(command: list[str], repeats: int) -> tuple[float, list[float]]:
    warmup = subprocess.run(command, capture_output=True, text=True)
    if warmup.returncode:
        raise RuntimeError(
            f"compiler exited with {warmup.returncode} during warmup\n"
            f"{' '.join(command)}\n{warmup.stdout}\n{warmup.stderr}"
        )

    samples: list[float] = []
    for _ in range(repeats):
        start = time.perf_counter()
        result = subprocess.run(command, capture_output=True, text=True)
        elapsed = (time.perf_counter() - start) * 1000.0
        if result.returncode:
            raise RuntimeError(
                f"compiler exited with {result.returncode}\n"
                f"{' '.join(command)}\n{result.stdout}\n{result.stderr}"
            )
        samples.append(elapsed)
    return statistics.median(samples), samples


def safe_stem(probe: Probe) -> str:
    raw = f"{probe.category}_{probe.case}_{probe.style}_{probe.count}"
    return "".join(character if character.isalnum() else "_" for character in raw)


def enforce_cxx20_gate(rows: list[dict[str, object]], args: argparse.Namespace) -> None:
    if args.scope not in {"summary", "all"} or not {20, 23}.issubset(args.standards):
        return

    largest = max(args.growth_counts)
    selected = {
        int(row["standard"]): row
        for row in rows
        if row["category"] == "expression-growth"
        and row["style"] == "expression"
        and row["functions"] == largest
        and row["mode"] == "optimized"
    }
    cxx20 = selected[20]
    cxx23 = selected[23]
    compile_regression = (
        float(cxx20["median_ms"]) / float(cxx23["median_ms"]) - 1.0
    ) * 100.0
    object_growth = (
        int(cxx20["object_bytes"]) / int(cxx23["object_bytes"]) - 1.0
    ) * 100.0
    print(
        f"C++20 optimized expression delta at {largest} functions: "
        f"compile={compile_regression:+.2f}% object={object_growth:+.2f}%",
        flush=True,
    )
    if compile_regression > args.max_compile_regression_percent:
        raise RuntimeError(
            f"C++20 optimized expression compile regression {compile_regression:.2f}% "
            f"exceeds {args.max_compile_regression_percent:.2f}%"
        )
    if object_growth > args.max_object_growth_percent:
        raise RuntimeError(
            f"C++20 optimized expression object growth {object_growth:.2f}% "
            f"exceeds {args.max_object_growth_percent:.2f}%"
        )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", type=Path, required=True)
    parser.add_argument("--compiler-id", required=True)
    parser.add_argument("--include", type=Path, required=True)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--scope", choices=("summary", "expressions", "all"), default="summary")
    parser.add_argument("--growth-counts", type=int, nargs="+", default=[25, 100, 200])
    parser.add_argument("--shape-count", type=int, default=64)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--standards", type=int, nargs="+", default=[20, 23])
    parser.add_argument("--max-compile-regression-percent", type=float, default=15.0)
    parser.add_argument("--max-object-growth-percent", type=float, default=5.0)
    args = parser.parse_args()

    if set(args.standards) - {20, 23}:
        parser.error("--standards accepts only 20 and 23")
    if args.repeats < 1 or args.shape_count < 1 or any(count < 1 for count in args.growth_counts):
        parser.error("repeat and function counts must be positive")

    if args.work.exists():
        shutil.rmtree(args.work)
    args.work.mkdir(parents=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)

    rows: list[dict[str, object]] = []
    for probe in probes_for(args.scope, args.growth_counts, args.shape_count):
        stem = safe_stem(probe)
        source = args.work / f"{stem}.cpp"
        source.write_text(source_for(probe), encoding="utf-8", newline="\n")
        for standard in args.standards:
            for mode in ("unoptimized", "optimized"):
                output = args.work / f"{stem}_cxx{standard}_{mode}.obj"
                command = compiler_command(
                    args.compiler,
                    args.compiler_id,
                    standard,
                    args.include,
                    source,
                    output,
                    mode == "optimized",
                )
                median_ms, samples = measure(command, args.repeats)
                row: dict[str, object] = {
                    "category": probe.category,
                    "case": probe.case,
                    "style": probe.style,
                    "standard": standard,
                    "mode": mode,
                    "functions": probe.count,
                    "median_ms": f"{median_ms:.3f}",
                    "samples_ms": "|".join(f"{sample:.3f}" for sample in samples),
                    "object_bytes": output.stat().st_size,
                }
                rows.append(row)
                print(
                    f"{probe.category:<18} {probe.case:<38} {probe.style:<10} "
                    f"C++{standard:<2} {mode:<11} {median_ms:9.1f} ms "
                    f"{output.stat().st_size:9d} bytes",
                    flush=True,
                )

    with args.output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    print(f"Wrote {args.output}", flush=True)
    enforce_cxx20_gate(rows, args)


if __name__ == "__main__":
    main()
