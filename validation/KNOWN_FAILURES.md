# Release findings

The release suite does not reduce thresholds to hide findings, and the
internal metrics runner never publishes canonical CSVs from a failing run.

## Current validation status

`fltx_ci_checks` runs the 4,096-sample standard accuracy corpus for f32, f64,
fdd, and fqd through both the normal and fixed-constexpr runners, in addition
to the contract, genuine-constexpr, dependency, in-tree package smoke, header,
standard, and tooling checks. On 12 September 2026, all seven available presets
passed at numerical source fingerprint
`56e43f70d7990d30d63a553087a8bb3eacfcc266675bad984cfc89eae872fdaa`:
Windows x64 MSVC, clang-cl and MinGW; Emscripten wasm32; Linux x64 GCC and
Clang; and macOS ARM64 AppleClang. This records the pre-audit baseline, not
subsequent source changes. Windows ARM64, Linux ARM64 and Intel Mac were not
available. The standalone installed-package fixture remains a separate target.
Runtime f32/f64 threshold findings are retained as advisory platform-libm
baseline evidence. All consumer-fast-math accuracy lanes are advisory because
reassociation and subnormal flushing can invalidate expansion arithmetic, and
runtime fixed simulation does not reproduce genuine compiler constant
evaluation. Genuine strict native numerical constant evaluation, native
special-value assertions in both consumer profiles, strict native fixed
simulation, and strict fdd/fqd accuracy remain gating.

The complete consumer-fast-math contract executable is compiled on every
configured target but is not a badge gate. GCC, Clang, AppleClang, MinGW, and
Emscripten legitimately produce different findings when their fast-math modes
discard signed zero, flush subnormals, assume finite inputs, or reassociate the
error-free transforms used by expansion arithmetic. Run that executable
directly when collecting diagnostic fast-math contract evidence.

Windows ARM64 presets disable the vendored TLFloat and qdpp comparisons because
their MSVC-compatible paths currently require x64-only integer or FMA
intrinsics. Windows x64 clang-cl also disables qdpp's baseline-unsafe FMA path.
These are explicit dependency support limits; FLTX's own ARM64 and clang-cl
validation remains enabled.

The isolated public-header probes emit undefined-inline warnings under
GCC and Clang-family compilers for declarations used by some single-header probes. They
are warning-level header-hygiene debt, not hidden test failures.

## Open conversion finding

Native floating casts currently add expansion limbs serially in `long double`
before the final conversion. This can lose a tail that determines rounding,
even for canonical storage. For example, on a host with 64-bit long-double
precision, converting `fdd_s{0x1.000001p0, 0x1p-80}` to `float` yields `1`
instead of `1 + 2^-23`. Related double and long-double witnesses fail in both
runtime and constant evaluation. Raw overlapping/cancelling limbs expose an
additional summation issue.

This remains an open pre-existing correctness finding. A correction needs one
shared target-aware rounding implementation, including overflow-before-
cancellation and subnormal handling; normalizing then retaining the same
serial sum does not fix it. The current audit does not claim correctly rounded
native floating conversion.

## Open exponential endpoint finding

At the declared exponential overflow cutoff and some immediately preceding
last-limb neighbors, the existing expansion kernel can overflow an
intermediate and produce noncanonical nonfinite results. Genuine constant
evaluation can reject the same intermediate. The audit reproduced this in
the committed baseline as well as the candidate.

`sinh` and `cosh` inherit this narrow endpoint defect through their exponential
calls, including near the shifted cutoff used for their added finite range.
The range extension does not establish exact rounding or classification at
every overflow-transition neighbor. Fixing the underlying exponential kernel
and its endpoint policy remains separate work.

## Open large-precision formatting finding

Some formatting routes assume that their requested precision fits fixed
internal scratch storage even when the caller provides a larger output buffer.
The audit reproduced scientific precision 384 terminating through a noexcept
writer and QD hexadecimal precision 600 reporting success with incorrect
digits. The width/precision parser now rejects integer accumulation overflow,
but that does not fix these separate numerical and scratch-capacity limits.

The resource policy remains undecided: support larger requests by generating
bounded exact digits and emitting additional zero padding, or document and
enforce a proven precision limit with explicit errors. Neither policy has
been implemented in this audit; arbitrary requested formatting precision is
not currently guaranteed.

## Preserved evidence

Failed accuracy runs are retained as `.partial.csv` files under their isolated
build trees. Each row records the seed, exact input limbs, observed result, and
MPFR result for its worst witness. The suffix itself makes no completeness
guarantee--an interrupted run uses it too--and a partial file is never a
publishable canonical result.
