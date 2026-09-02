#!/usr/bin/env python3
"""Write deterministic executable-size deltas for fltx probes."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def measured_path(path: Path) -> Path:
    wasm = path.with_suffix(".wasm")
    return wasm if wasm.exists() else path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--file", action="append", default=[], metavar="NAME=PATH")
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    rows = []
    sizes: dict[str, int] = {}
    for item in args.file:
        name, raw_path = item.split("=", 1)
        path = measured_path(Path(raw_path))
        if not path.exists():
            raise FileNotFoundError(path)
        sizes[name] = path.stat().st_size

    for name, size in sizes.items():
        precision = "dd" if "_dd_" in name else "qd"
        baseline = sizes[f"fltx_size_{precision}_baseline"]
        rows.append(
            {
                "precision": precision,
                "case": name.removeprefix(f"fltx_size_{precision}_"),
                "bytes": size,
                "delta_bytes": size - baseline,
            }
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    for row in rows:
        print(
            f"{row['precision']} {row['case']:<12} "
            f"{row['bytes']:>9} bytes ({row['delta_bytes']:+d})"
        )
    print(f"Wrote {args.output}")


if __name__ == "__main__":
    main()
