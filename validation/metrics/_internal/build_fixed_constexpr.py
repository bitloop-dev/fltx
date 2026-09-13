"""Render validated fixed constexpr accuracy evidence, with one row per domain."""

from __future__ import annotations

import uuid
from pathlib import Path
from typing import Mapping

from build_tables import GROUP_LABELS, GROUP_ORDER, OPERATION_ORDER, Target, _target
from build_overview import _fmt_bits
from manifest import normalize_operations
import run_metrics
import run_native_accuracy as accuracy
from svg_table import Cell, Column, Row, Table, render


def load_run(metadata_path: Path, consumer_mode: str) -> tuple[dict, dict]:
    """Verify the complete evidence set before a renderer consumes any rows."""
    metadata = run_metrics._read_metadata(metadata_path)
    try:
        configuration = run_metrics.require_consumer_mode(
            metadata["configuration"], consumer_mode, metadata_path,
        )
        target = run_metrics.infer_canonical_target(configuration, metadata_path)
        operations = normalize_operations(metadata.get("operations", ()))
        result = accuracy._validate_reusable_candidate(
            metadata_path,
            sample_mode=metadata["sample_mode"], consumer_mode=consumer_mode,
            platform=target[0], architecture=target[1], compiler=target[2],
            fingerprint=metadata["source_fingerprint"],
            configuration=configuration, host=metadata["host"],
            operations=operations, fixed_constexpr=True,
        )
    except (KeyError, TypeError, ValueError) as error:
        raise run_metrics.MetricsError(f"{metadata_path}: invalid fixed constexpr metadata: {error}") from error
    return metadata, result


def render_precision(rows: list[dict[str, str]], metadata: dict, precision: str) -> str:
    def order(row):
        group, operation = row["group"], row["operation"]
        return (GROUP_ORDER.get(group, 999), group,
                OPERATION_ORDER.get(group, {}).get(operation, 999), operation, row["domain"])

    table_rows = []
    for row in sorted(rows, key=order):
        passed = row["pass"] == "yes"
        tooltip = (
            f"{row['api']} | samples: {row['samples']} | seed: {row['seed']}\n"
            f"Worst input: {row['worst_input']}\n"
            f"Observed: {row['observed']}\nMPFR: {row['reference']}"
        )
        table_rows.append(Row(
            GROUP_LABELS.get(row["group"], row["group"]),
            f"{row['operation']} / {row['domain']}",
            (
                *(Cell((_fmt_bits(row[field]),), tooltip=tooltip) for field in
                  ("mean_bits", "p01_bits", "worst_bits", "required_worst_bits")),
                Cell(("PASS" if passed else "FAIL",), fill="#dcfce7" if passed else "#fee2e2", tooltip=tooltip),
                Cell((row["special_support"],)),
                Cell((row["signed_zero_support"],)),
            ),
        ))
    target = Target(metadata["platform"], metadata["architecture"], metadata["compiler"])
    legend = [
        "Each row is one MPFR accuracy domain; exact means every sampled result was exact.",
        f"Thresholds: {metadata['thresholds']}. Signed zero is reported independently of numerical accuracy.",
        f"Run: {metadata['run_id']} | Source: {metadata['source_fingerprint'][:16]}",
    ]
    if metadata["consumer_mode"] == "fastmath":
        legend.append("Inf/NaN probes are omitted under fixed constexpr fast-math; '-' means untested or inapplicable.")
    return render(Table(
        title=f"FLTX {precision} fixed constexpr accuracy",
        description=f"{target.label} | {metadata['consumer_mode']} | {metadata['sample_mode']} | constexpr algorithms executed at runtime",
        columns=(Column("Mean", "bits"), Column("p01", "bits"), Column("Worst", "bits"),
                 Column("Required", "worst bits"), Column("Threshold"), Column("Inf / NaN"), Column("Signed zero")),
        rows=tuple(table_rows), legend=tuple(legend),
    ))


def build_run(
    metadata_path: Path,
    output: Path,
    consumer_mode: str,
    *,
    expected_result: Mapping[str, object] | None = None,
) -> list[Path]:
    metadata, result = load_run(metadata_path, consumer_mode)
    if expected_result is not None and dict(expected_result) != result:
        raise run_metrics.MetricsError(f"{metadata_path}: fixed constexpr result handoff disagrees with evidence")
    root = Path(__file__).resolve().parents[3]
    if metadata.get("operations") and not output.resolve().is_relative_to(root / "build"):
        raise run_metrics.MetricsError("operation-filtered reports must stay under build/")
    canonical = root / "validation" / "metrics" / "generated"
    if output.resolve().is_relative_to(canonical):
        if metadata["sample_mode"] != "full" or metadata["source_revision"] == "unknown":
            raise run_metrics.MetricsError("fixed constexpr publication requires complete full-profile evidence and a known revision")
        run_metrics._validate_publishable_build(metadata["configuration"], metadata_path)

    target = _target(result["target"])
    prefix = f"{target.platform}_{target.architecture}_{target.compiler}"
    suffix = "_fixed_constexpr" + run_metrics.consumer_mode_suffix(consumer_mode)
    staging = output / ".staging" / uuid.uuid4().hex
    staging.mkdir(parents=True, exist_ok=False)
    staged_outputs = []
    for precision in metadata["precisions"]:
        source = metadata_path.parent / accuracy.evidence_name(precision, True)
        rows = run_metrics._read_csv(source, run_metrics.ACCURACY_FIELDS)
        name = f"{prefix}_{precision}{suffix}_overview.svg"
        staged = staging / name
        staged.write_text(render_precision(rows, metadata, precision), encoding="utf-8", newline="\n")
        staged_outputs.append((staged, output / "overview" / name))
    marker = f"{prefix}{suffix}_reports.json"
    staged_metadata = staging / marker
    run_metrics._write_json(staged_metadata, {
        "schema_version": 1, "execution_profile": "fixed_constexpr",
        "run_id": metadata["run_id"], "source_fingerprint": metadata["source_fingerprint"],
        "sample_mode": metadata["sample_mode"], "consumer_mode": consumer_mode,
        "evidence": str(metadata_path.resolve()),
        "evidence_sha256": run_metrics._sha256_file(metadata_path),
        "outputs": {final.name: run_metrics._sha256_file(staged) for staged, final in staged_outputs},
    })
    with run_metrics._publication_lock(output / "overview" / f".{prefix}{suffix}.publish.lock"):
        run_metrics._publish_transaction(staged_outputs, staged_metadata, output / "overview" / marker)
    return [final for _, final in staged_outputs]


def build(
    input_root: Path,
    output: Path,
    targets: tuple[Target, ...] | None,
    consumer_mode: str,
) -> list[Path]:
    candidates = sorted(
        (input_root / "fixed_constexpr" / consumer_mode).glob(f"*/{accuracy.metadata_name(True)}"),
        key=lambda path: path.stat().st_mtime_ns, reverse=True,
    )
    selected = {}
    for path in candidates:
        metadata = run_metrics._read_metadata(path)
        target = _target(str(metadata.get("target", "")))
        if targets is None or target in targets:
            selected.setdefault(target, path)
    if not selected or (targets is not None and set(selected) != set(targets)):
        raise run_metrics.MetricsError(f"missing complete fixed constexpr evidence under {input_root}")
    # Validate every selected target before producing any report.
    for path in selected.values():
        load_run(path, consumer_mode)
    return [report for path in selected.values() for report in build_run(path, output, consumer_mode)]
