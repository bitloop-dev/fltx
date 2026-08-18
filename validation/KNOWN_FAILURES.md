# Release findings

The release suite does not reduce thresholds to hide findings, and the
internal metrics runner never publishes canonical CSVs from a failing run.

## Current validation status

`fltx_ci_checks` runs the 65,536-sample accuracy corpus for f32, f64, f128,
and f256 through both the normal and fixed-constexpr runners, in addition to
the contract, genuine-constexpr, dependency, in-tree package smoke, header,
standard, and tooling checks. A same-fingerprint result for this expanded
badge gate has not yet been recorded, so this file makes no passing-platform
claim. The standalone installed-package fixture remains a separate target.
Runtime f32/f64 threshold findings are retained as advisory platform-libm
baseline evidence. Native fixed-simulation consumer-fast-math findings are also
advisory because runtime code generation does not reproduce genuine compiler
constant evaluation. Genuine strict native numerical constant evaluation,
native special-value assertions in both consumer profiles, strict native fixed-
simulation, and every f128/f256 threshold remain gating.

The isolated public-header probes emit `undefined-inline` warnings under
MinGW and Emscripten for declarations used by some single-header probes. They
are warning-level header-hygiene debt, not hidden test failures.

## Preserved evidence

Failed accuracy runs are retained as `.partial.csv` files under their isolated
build trees. Each row records the seed, exact input limbs, observed result, and
MPFR result for its worst witness. The suffix itself makes no completeness
guarantee--an interrupted run uses it too--and a partial file is never a
publishable canonical result.
