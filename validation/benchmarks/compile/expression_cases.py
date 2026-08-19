"""Declarative f256 expression-shape manifest used by compile benchmarks."""

from __future__ import annotations


EXPRESSION_CASES: dict[str, str] = {
    "pass": "a",
    "add": "a + b",
    "sub": "a - b",
    "mul": "a * b",
    "div": "a / b",
    "mul_add": "a * b + c",
    "mul_sub": "a * b - c",
    "value_sub_mul": "c - a * b",
    "mul_add_mul": "a * b + c * d",
    "mul_sub_mul": "a * b - c * d",
    "mul_add_mul_add": "a * b + c * d + e",
    "mul_add_mul_sub": "a * b + c * d - e",
    "mul_sub_mul_add": "a * b - c * d + e",
    "mul_sub_mul_sub": "a * b - c * d - e",
    "mul_add_add": "a * b + c + d",
    "mul_add_sub": "a * b + c - d",
    "mul_sub_add": "a * b - c + d",
    "mul_sub_sub": "a * b - c - d",
    "three_products": "a * b + c * d + e * f",
    "four_products": "a * b + c * d + e * f + g * h",
    "add_mul_double": "a + b * s",
    "sub_mul_double": "a - b * s",
    "mul_double_sub": "a * s - b",
    "mul_double_add_mul_double": "a * s + b * t",
    "mul_double_add_mul_double_add": "a * s + b * t + c",
    "mul_add_div": "(a * b + c) / d",
    "mul_sub_div": "(a * b - c) / d",
    "value_sub_mul_div": "(c - a * b) / d",
    "mul_add_mul_div": "(a * b + c * d) / e",
    "mul_sub_mul_div": "(a * b - c * d) / e",
    "add_add_sub_div": "(a + b - c) / d",
    "add_sub_sub_div": "(a - b - c) / d",
    "add_mul_double_div": "(a + b * s) / d",
    "sub_mul_double_div": "(a - b * s) / d",
    "mul_double_sub_div": "(a * s - b) / d",
    "div_add_double": "a / (b + s)",
    "div_double_sub": "a / (s - b)",
    "mul_add_div_add_double": "(a * b + c) / (d + s)",
    "mul_sub_div_add_double": "(a * b - c) / (d + s)",
    "value_sub_mul_div_add_double": "(c - a * b) / (d + s)",
    "mul_add_mul_div_add_double": "(a * b + c * d) / (e + s)",
    "mul_sub_mul_div_add_double": "(a * b - c * d) / (e + s)",
    "add_add_add_div_add_double": "(a + b + c) / (d + s)",
    "add_sub_add_div_add_double": "(a - b + c) / (d + s)",
    "add_add_sub_div_add_double": "(a + b - c) / (d + s)",
    "add_sub_sub_div_add_double": "(a - b - c) / (d + s)",
    "add_mul_double_div_add_double": "(a + b * s) / (d + t)",
    "sub_mul_double_div_add_double": "(a - b * s) / (d + t)",
    "mul_double_sub_div_add_double": "(a * s - b) / (d + t)",
}

EXPRESSION_STRESS_CASES = (*EXPRESSION_CASES, "all_supported_fused_kernels")


def validate_manifest() -> None:
    if len(EXPRESSION_CASES) != 49:
        raise ValueError(
            f"expected 49 individual f256 expression shapes, found {len(EXPRESSION_CASES)}"
        )
    if len(EXPRESSION_STRESS_CASES) != 50:
        raise ValueError("the expression benchmark must define 50 stress cases")
    for case_id, expression in EXPRESSION_CASES.items():
        if not case_id.isidentifier() or not expression.strip():
            raise ValueError(f"invalid expression case {case_id!r}")


def expression_for(case_id: str, index: int = 0) -> str:
    if case_id == "all_supported_fused_kernels":
        expressions = tuple(EXPRESSION_CASES.values())
        return expressions[index % len(expressions)]
    try:
        return EXPRESSION_CASES[case_id]
    except KeyError as error:
        raise ValueError(f"unknown f256 expression case {case_id!r}") from error


validate_manifest()
