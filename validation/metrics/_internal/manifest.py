"""Explicit release manifests for the metrics implementations."""

from __future__ import annotations

import hashlib
from collections.abc import Iterable
from dataclasses import dataclass
from typing import TypeVar


Operation = tuple[str, str]
AccuracyRow = tuple[str, str, str]
_Row = TypeVar("_Row", bound=tuple)


@dataclass(frozen=True)
class Implementation:
    id: str
    short_label: str
    label: str
    nominal_bits: float


def _accuracy_rows(*, paired_arithmetic: bool) -> frozenset[AccuracyRow]:
    arithmetic = {
        operation: ("general", "subnormal")
        for operation in ("add", "subtract", "multiply", "divide")
    } if paired_arithmetic else {
        "add": (
            "near_one", "moderate", "wide_exponent", "extreme_finite", "subnormal",
        ),
        "subtract": ("near_one", "cancellation", "subnormal"),
        "multiply": ("near_one", "moderate", "wide_exponent", "extreme_finite"),
        "divide": ("near_one", "moderate", "wide_exponent", "extreme_finite"),
    }
    operations = {
        ("arithmetic", operation): domains
        for operation, domains in arithmetic.items()
    } | {
        ("io", "to_string"): ("moderate", "wide_exponent"),
        ("io", "to_chars"): ("moderate", "wide_exponent"),
        ("io", "parse"): ("moderate", "wide_exponent"),
        ("floating_point_utilities", "fma"): ("moderate", "cancellation", "wide_exponent"),
        ("floating_point_utilities", "abs"): ("wide_exponent", "subnormal"),
        ("floating_point_utilities", "fabs"): ("moderate",),
        ("roots_and_powers", "sqr"): ("moderate", "wide_exponent"),
        ("floating_point_utilities", "recip"): ("near_one", "wide_exponent"),
        ("floating_point_utilities", "fmin"): ("moderate", "wide_exponent"),
        ("floating_point_utilities", "fmax"): ("moderate", "wide_exponent"),
        ("floating_point_utilities", "fdim"): ("moderate", "cancellation"),
        ("floating_point_utilities", "copysign"): ("moderate",),
        ("rounding", "floor"): ("boundary",),
        ("rounding", "ceil"): ("boundary",),
        ("rounding", "trunc"): ("boundary",),
        ("rounding", "round"): ("boundary",),
        ("rounding", "roundeven"): ("boundary",),
        ("rounding", "lround"): ("boundary",),
        ("rounding", "llround"): ("boundary",),
        ("rounding", "round_decimals"): ("moderate",),
        ("rounding", "round_significant"): ("wide_exponent",),
        ("remainders", "remquo"): ("moderate",),
        ("floating_point_utilities", "modf"): ("moderate", "wide_exponent"),
        ("floating_point_utilities", "ldexp"): ("wide_exponent",),
        ("floating_point_utilities", "scalbn"): ("wide_exponent",),
        ("floating_point_utilities", "scalbln"): ("wide_exponent",),
        ("floating_point_utilities", "frexp"): ("wide_exponent", "subnormal"),
        ("floating_point_utilities", "ilogb"): ("moderate", "wide_exponent"),
        ("floating_point_utilities", "logb"): ("moderate", "wide_exponent"),
        ("roots_and_powers", "sqrt"): ("near_one", "wide_exponent", "extreme_finite", "subnormal"),
        ("roots_and_powers", "cbrt"): ("moderate", "wide_exponent"),
        ("roots_and_powers", "hypot"): ("moderate", "wide_exponent"),
        ("trigonometric", "sin"): (
            "moderate", "argument_reduction", "quadrant_boundaries",
        ),
        ("trigonometric", "cos"): (
            "moderate", "argument_reduction", "quadrant_boundaries",
        ),
        ("trigonometric", "tan"): (
            "moderate", "argument_reduction", "quadrant_boundaries",
        ),
        ("trigonometric", "atan"): ("near_one", "moderate"),
        ("trigonometric", "atan2"): ("near_one", "moderate", "extreme_finite"),
        ("trigonometric", "asin"): ("moderate", "boundary"),
        ("trigonometric", "acos"): ("moderate", "boundary"),
        ("exponentials", "exp"): ("moderate", "boundary"),
        ("exponentials", "exp2"): ("moderate", "boundary"),
        ("exponentials", "expm1"): ("moderate", "cancellation"),
        ("logarithms", "log"): ("near_one", "wide_exponent", "extreme_finite"),
        ("logarithms", "log2"): ("near_one", "wide_exponent", "extreme_finite"),
        ("logarithms", "log10"): ("near_one", "wide_exponent", "extreme_finite"),
        ("logarithms", "log1p"): ("moderate", "cancellation"),
        ("roots_and_powers", "pow"): ("near_one", "moderate"),
        ("roots_and_powers", "ipow"): ("moderate", "wide_exponent"),
        ("remainders", "fmod"): (
            "moderate",
            "wide_exponent",
        ) + (("quotient_reduction",) if paired_arithmetic else ()),
        ("remainders", "remainder"): ("moderate",),
        ("hyperbolic", "sinh"): ("near_one", "moderate"),
        ("hyperbolic", "cosh"): ("near_one", "moderate"),
        ("hyperbolic", "tanh"): ("near_one", "moderate"),
        ("inverse_hyperbolic", "asinh"): ("near_one", "moderate"),
        ("inverse_hyperbolic", "acosh"): ("moderate", "boundary"),
        ("inverse_hyperbolic", "atanh"): ("moderate", "boundary"),
        ("special_functions", "erf"): ("near_one", "moderate"),
        ("special_functions", "erfc"): ("near_one", "moderate"),
        ("special_functions", "lgamma"): ("moderate",),
        ("special_functions", "tgamma"): ("moderate",),
    }
    return frozenset(
        (group, operation, domain)
        for (group, operation), domains in operations.items()
        for domain in domains
    )


_ALL_ACCURACY = _accuracy_rows(paired_arithmetic=True)
_NATIVE_ACCURACY = _accuracy_rows(paired_arithmetic=False)
_NATIVE_EXCLUDED_ACCURACY = frozenset({
    ("io", "to_string", "moderate"),
    ("io", "to_string", "wide_exponent"),
    ("io", "to_chars", "moderate"),
    ("io", "to_chars", "wide_exponent"),
    ("io", "parse", "moderate"),
    ("io", "parse", "wide_exponent"),
    ("trigonometric", "sin", "quadrant_boundaries"),
    ("trigonometric", "cos", "quadrant_boundaries"),
    ("trigonometric", "tan", "quadrant_boundaries"),
})
_COMPARISON_ACCURACY = frozenset({
    ("comparisons", operation, "moderate")
    for operation in (
        "equal", "not_equal", "less", "less_equal", "greater", "greater_equal",
    )
})
EXPECTED_ACCURACY = {
    "f32": _NATIVE_ACCURACY - _NATIVE_EXCLUDED_ACCURACY,
    "f64": _NATIVE_ACCURACY - _NATIVE_EXCLUDED_ACCURACY,
    "dd": _ALL_ACCURACY | _COMPARISON_ACCURACY,
    "qd": _ALL_ACCURACY | _COMPARISON_ACCURACY,
}

_BENCHMARK_OPERATIONS = frozenset({
    ("arithmetic", "add"), ("arithmetic", "subtract"),
    ("arithmetic", "multiply"), ("arithmetic", "divide"),
    ("floating_point_utilities", "fma"), ("floating_point_utilities", "abs"), ("floating_point_utilities", "fabs"),
    ("roots_and_powers", "sqr"), ("floating_point_utilities", "recip"), ("floating_point_utilities", "fmin"),
    ("floating_point_utilities", "fmax"), ("floating_point_utilities", "fdim"), ("floating_point_utilities", "copysign"),
    ("rounding", "floor"), ("rounding", "ceil"), ("rounding", "trunc"),
    ("rounding", "round"), ("rounding", "roundeven"), ("rounding", "lround"),
    ("rounding", "llround"), ("rounding", "round_decimals"),
    ("rounding", "round_significant"),
    ("remainders", "fmod"), ("remainders", "remainder"), ("remainders", "remquo"),
    ("floating_point_utilities", "modf"), ("floating_point_utilities", "ldexp"),
    ("floating_point_utilities", "scalbn"), ("floating_point_utilities", "scalbln"),
    ("floating_point_utilities", "frexp"), ("floating_point_utilities", "ilogb"),
    ("floating_point_utilities", "logb"), ("floating_point_utilities", "nextafter"),
    ("floating_point_utilities", "nexttoward"), ("comparisons", "equal"),
    ("comparisons", "not_equal"), ("comparisons", "less"),
    ("comparisons", "less_equal"), ("comparisons", "greater"),
    ("comparisons", "greater_equal"), ("roots_and_powers", "sqrt"), ("roots_and_powers", "cbrt"),
    ("roots_and_powers", "hypot"), ("trigonometric", "sin"), ("trigonometric", "cos"), ("trigonometric", "sincos"),
    ("trigonometric", "tan"), ("trigonometric", "atan"), ("trigonometric", "atan2"), ("trigonometric", "asin"),
    ("trigonometric", "acos"), ("exponentials", "exp"), ("exponentials", "exp2"),
    ("exponentials", "expm1"), ("logarithms", "log"), ("logarithms", "log2"),
    ("logarithms", "log10"), ("logarithms", "log1p"), ("roots_and_powers", "pow"),
    ("roots_and_powers", "ipow"), ("mixed_workloads", "product_sum"),
    ("hyperbolic", "sinh"), ("hyperbolic", "cosh"), ("hyperbolic", "tanh"),
    ("inverse_hyperbolic", "asinh"), ("inverse_hyperbolic", "acosh"), ("inverse_hyperbolic", "atanh"),
    ("special_functions", "erf"), ("special_functions", "erfc"), ("special_functions", "lgamma"),
    ("special_functions", "tgamma"), ("io", "to_string"), ("io", "to_chars"),
    ("io", "parse"), ("random", "mt19937_64"), ("random", "uniform_real"),
    ("random", "normal"), ("mixed_workloads", "chained_arithmetic"),
    ("mixed_workloads", "affine_trig"), ("mixed_workloads", "mandelbrot"),
    ("mixed_workloads", "matrix_vector_4x4"),
    ("mixed_workloads", "horner_polynomial"),
    ("mixed_workloads", "newton_root"),
    ("mixed_workloads", "log_sum_exp"),
    ("mixed_workloads", "haversine"),
})

EXPECTED_BENCHMARK = {
    precision: _BENCHMARK_OPERATIONS
    for precision in ("dd", "qd")
}


def normalize_operations(values: Iterable[str] | None) -> tuple[str, ...]:
    """Validate exact operation names and give equivalent selections one identity."""
    operations = tuple(sorted(set(values or ())))
    available = {operation for _, operation in _BENCHMARK_OPERATIONS}
    unknown = set(operations) - available
    if unknown:
        raise ValueError(
            f"unknown metrics operations: {', '.join(repr(item) for item in sorted(unknown))}; "
            f"available: {', '.join(sorted(available))}"
        )
    return operations


def select_operations(
    rows: Iterable[_Row], operations: Iterable[str] = (),
) -> frozenset[_Row]:
    selected = frozenset(operations)
    return frozenset(row for row in rows if not selected or row[1] in selected)


def operation_key(operations: tuple[str, ...]) -> str:
    """Keep development directory names readable and bounded on Windows."""
    name = "+".join(normalize_operations(operations))
    if len(name) > 64:
        return name[:32] + "-" + hashlib.sha256(name.encode()).hexdigest()[:16]
    return name

_NO_QDPP = frozenset({
    ("floating_point_utilities", "fma"), ("floating_point_utilities", "fdim"), ("floating_point_utilities", "copysign"),
    ("rounding", "roundeven"), ("rounding", "lround"), ("rounding", "llround"),
    ("rounding", "round_decimals"), ("rounding", "round_significant"),
    ("floating_point_utilities", "modf"), ("floating_point_utilities", "frexp"),
    ("floating_point_utilities", "ilogb"), ("floating_point_utilities", "logb"),
    ("floating_point_utilities", "nextafter"), ("floating_point_utilities", "nexttoward"), ("roots_and_powers", "hypot"),
    ("exponentials", "exp2"), ("logarithms", "log2"), ("special_functions", "erf"),
    ("special_functions", "erfc"), ("special_functions", "lgamma"), ("special_functions", "tgamma"),
    ("random", "mt19937_64"), ("random", "normal"),
})

_BOOST_UNAVAILABLE = frozenset({
    ("roots_and_powers", "sqr"), ("floating_point_utilities", "recip"), ("rounding", "roundeven"),
    ("rounding", "round_decimals"), ("rounding", "round_significant"),
    ("trigonometric", "sincos"), ("io", "to_chars"), ("random", "mt19937_64"),
    ("random", "uniform_real"), ("random", "normal"),
})

_TLFLOAT_UNAVAILABLE = frozenset({
    ("roots_and_powers", "sqr"), ("floating_point_utilities", "recip"),
    ("rounding", "lround"), ("rounding", "llround"),
    ("rounding", "round_decimals"),
    ("rounding", "round_significant"),
    ("floating_point_utilities", "scalbn"), ("floating_point_utilities", "scalbln"),
    ("floating_point_utilities", "logb"), ("roots_and_powers", "ipow"), ("io", "to_chars"),
    ("random", "mt19937_64"), ("random", "uniform_real"),
    ("random", "normal"),
})

IMPLEMENTATIONS = {
    "dd": (
        Implementation("fltx", "fltx", "fltx bl::fdd", 106.0),
        Implementation("qdpp", "ddreal", "qdpp dd_real", 106.0),
        Implementation(
            "cppdd", "cppdd", "boost::multiprecision::cpp_double_double", 106.0,
        ),
        Implementation("tlfloat", "tlquad", "TLFloat Quad", 113.0),
    ),
    "qd": (
        Implementation("fltx", "fltx", "fltx bl::fqd", 212.0),
        Implementation("qdpp", "qdreal", "qdpp qd_real", 212.0),
        Implementation(
            "mpfr64", "mpfr64",
            "boost::multiprecision::mpfr_float_backend<64>",
            214.0,
        ),
        Implementation("tlfloat", "tloct", "TLFloat Octuple", 237.0),
    ),
}


def enabled_implementations(
    precision: str, *, qdpp: bool, tlfloat: bool,
) -> tuple[Implementation, ...]:
    return tuple(
        implementation
        for implementation in IMPLEMENTATIONS[precision]
        if (qdpp or implementation.id != "qdpp")
        and (tlfloat or implementation.id != "tlfloat")
    )


def benchmark_manifest(
    precision: str, *, qdpp: bool, tlfloat: bool,
) -> dict[str, frozenset[Operation]]:
    qd_missing = set(_NO_QDPP)
    if precision == "dd":
        qd_missing.update({("floating_point_utilities", "fmin"), ("floating_point_utilities", "fmax")})
    return {
        implementation.id: (
            EXPECTED_BENCHMARK[precision]
            if implementation.id == "fltx"
            else EXPECTED_BENCHMARK[precision] - qd_missing
            if implementation.id == "qdpp"
            else EXPECTED_BENCHMARK[precision] - _TLFLOAT_UNAVAILABLE
            if implementation.id == "tlfloat"
            else EXPECTED_BENCHMARK[precision] - _BOOST_UNAVAILABLE
        )
        for implementation in enabled_implementations(
            precision, qdpp=qdpp, tlfloat=tlfloat,
        )
    }


def accuracy_manifest(
    precision: str, *, qdpp: bool, tlfloat: bool,
) -> dict[str, frozenset[AccuracyRow]]:
    benchmark = benchmark_manifest(
        precision, qdpp=qdpp, tlfloat=tlfloat,
    )
    return {
        implementation: frozenset(
            row for row in EXPECTED_ACCURACY[precision] if row[:2] in operations
        )
        for implementation, operations in benchmark.items()
    }


def api_name(
    implementation: str,
    precision: str,
    group: str,
    operation: str,
) -> str:
    """Return stable noteworthy API labels used by fixture tests and reports."""

    if implementation == "fltx":
        return f"bl::{operation}"
    if implementation == "qdpp":
        special = {
            ("remainders", "remquo"): "qdpp divrem",
            ("io", "to_string"): "qdpp to_string",
            ("io", "to_chars"): "qdpp write",
            ("io", "parse"): "qdpp read",
            ("random", "uniform_real"):
                "qdpp ddrand" if precision == "dd" else "qdpp qdrand",
            ("floating_point_utilities", "recip"): "qdpp inv",
            ("roots_and_powers", "sqr"): "qdpp sqr",
        }
        return special.get((group, operation), f"qdpp {operation}")
    if implementation == "tlfloat":
        special = {
            ("arithmetic", "add"): "TLFloat operator+",
            ("arithmetic", "subtract"): "TLFloat operator-",
            ("arithmetic", "multiply"): "TLFloat operator*",
            ("arithmetic", "divide"): "TLFloat operator/",
            ("floating_point_utilities", "abs"): "tlfloat::fabs",
            ("floating_point_utilities", "fabs"): "tlfloat::fabs",
            ("rounding", "roundeven"): "tlfloat::rint",
            ("floating_point_utilities", "nexttoward"): "tlfloat::nextafter",
            ("comparisons", "equal"): "TLFloat operator==",
            ("comparisons", "not_equal"): "TLFloat operator!=",
            ("comparisons", "less"): "TLFloat operator<",
            ("comparisons", "less_equal"): "TLFloat operator<=",
            ("comparisons", "greater"): "TLFloat operator>",
            ("comparisons", "greater_equal"): "TLFloat operator>=",
            ("mixed_workloads", "product_sum"): "same expression",
            ("mixed_workloads", "chained_arithmetic"): "same workload",
            ("mixed_workloads", "affine_trig"): "same workload",
            ("mixed_workloads", "mandelbrot"): "same workload",
            ("mixed_workloads", "matrix_vector_4x4"): "same workload",
            ("mixed_workloads", "horner_polynomial"): "same workload",
            ("mixed_workloads", "newton_root"): "same workload",
            ("mixed_workloads", "log_sum_exp"): "same workload",
            ("mixed_workloads", "haversine"): "same workload",
            ("io", "to_string"): "tlfloat::to_string",
            ("io", "parse"): "TLFloat string constructor",
        }
        return special.get((group, operation), f"tlfloat::{operation}")
    boost = "boost::multiprecision"
    special = {
        ("roots_and_powers", "ipow"): f"{boost}::pow(value, int)",
        ("floating_point_utilities", "nexttoward"): "boost::math::nextafter",
        ("io", "to_string"): f"{boost}::number::str",
        ("io", "parse"): f"{boost} string constructor",
    }
    return special.get((group, operation), f"{boost}::{operation}")


assert len(EXPECTED_BENCHMARK["dd"]) == 85
assert len(EXPECTED_ACCURACY["f32"]) == 119
assert len(EXPECTED_ACCURACY["f64"]) == 119
_ALL = {"qdpp": True, "tlfloat": True}
assert len(benchmark_manifest("dd", **_ALL)["qdpp"]) == 60
assert len(benchmark_manifest("qd", **_ALL)["qdpp"]) == 62
assert len(benchmark_manifest("dd", **_ALL)["cppdd"]) == 75
assert len(benchmark_manifest("qd", **_ALL)["mpfr64"]) == 75
assert len(benchmark_manifest("dd", **_ALL)["tlfloat"]) == 71
assert len(benchmark_manifest("qd", **_ALL)["tlfloat"]) == 71
assert len(accuracy_manifest("dd", **_ALL)["fltx"]) == 127
assert len(accuracy_manifest("dd", **_ALL)["qdpp"]) == 91
assert len(accuracy_manifest("qd", **_ALL)["qdpp"]) == 95
assert len(accuracy_manifest("dd", **_ALL)["cppdd"]) == 118
assert len(accuracy_manifest("qd", **_ALL)["mpfr64"]) == 118
assert len(accuracy_manifest("dd", **_ALL)["tlfloat"]) == 111
assert len(accuracy_manifest("qd", **_ALL)["tlfloat"]) == 111
