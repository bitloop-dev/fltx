"""Aggregate report generation from existing complete metrics data."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import build_overview
import build_performance
import build_tables
from run_metrics import CONSUMER_MODES, MetricsError, consumer_mode_suffix


def _available_targets(
    input_root: Path,
    consumer_mode: str,
) -> tuple[build_tables.Target, ...]:
    dataset = build_performance.discover(input_root, consumer_mode)
    return tuple(dict.fromkeys(column.target for column in dataset.columns))


def rebuild(
    input_root: Path,
    output: Path,
    targets: tuple[build_tables.Target, ...] | None = None,
    consumer_mode: str = "strict",
) -> list[Path]:
    selected = targets or _available_targets(input_root, consumer_mode)
    if not selected:
        raise MetricsError(f"no complete metrics data found under {input_root}")

    datasets = {
        target: build_tables.load(input_root, (target,), consumer_mode)
        for target in selected
    }
    if any(dataset.operations for dataset in datasets.values()):
        build_root = Path(__file__).resolve().parents[3] / "build"
        if not output.resolve().is_relative_to(build_root):
            raise MetricsError("operation-filtered reports must stay under build/")

    output.mkdir(parents=True, exist_ok=True)
    outputs = list(
        build_tables.build(
            input_root,
            selected,
            output / "accuracy",
            consumer_mode,
        )
    )
    for layout in ("full", "compact"):
        outputs.extend(
            build_performance.build(
                input_root,
                output / "performance",
                layout,
                consumer_mode,
            )
        )

    for target, dataset in datasets.items():
        for layout in ("full", "compact"):
            mode_suffix = consumer_mode_suffix(consumer_mode)
            layout_suffix = "_compact" if layout == "compact" else ""
            for precision in build_tables.PRECISIONS:
                path = (
                    output
                    / "overview"
                    / (
                        f"{target.platform}_{target.architecture}_{target.compiler}_{precision}"
                        f"{mode_suffix}_overview{layout_suffix}.svg"
                    )
                )
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(
                    build_overview.render_overview(
                        dataset,
                        target,
                        precision,
                        layout,
                    ),
                    encoding="utf-8",
                    newline="\n",
                )
                outputs.append(path)
    return outputs


def parse_args(
    metrics_dir: Path,
    argv: list[str] | None = None,
) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input",
        type=Path,
        default=metrics_dir / "data",
        help="metrics data directory (default: validation/metrics/data)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=metrics_dir / "generated",
        help="report output directory (default: validation/metrics/generated)",
    )
    parser.add_argument(
        "--targets",
        type=build_tables._target,
        nargs="+",
        help=(
            "optional platform/architecture/compiler targets; defaults to all "
            "available data"
        ),
    )
    parser.add_argument(
        "--consumer-mode",
        choices=CONSUMER_MODES,
        default="strict",
    )
    return parser.parse_args(argv)


def main(metrics_dir: Path, argv: list[str] | None = None) -> int:
    args = parse_args(metrics_dir, argv)
    try:
        outputs = rebuild(
            args.input,
            args.output,
            tuple(args.targets) if args.targets else None,
            args.consumer_mode,
        )
    except (MetricsError, OSError, ValueError) as error:
        print(f"table rebuild failed: {error}", file=sys.stderr)
        return 1
    for output in outputs:
        print(output)
    return 0
